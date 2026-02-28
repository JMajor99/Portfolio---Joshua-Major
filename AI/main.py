import tkinter as tk
from pathlib import Path
from dotenv import load_dotenv
from pydantic import BaseModel
from langchain_openai import ChatOpenAI
from langchain_anthropic import ChatAnthropic
from langchain_core.prompts import ChatPromptTemplate
from langchain_core.output_parsers import PydanticOutputParser

env_path = Path(__file__).resolve().parent / ".env"
load_dotenv(dotenv_path=env_path) # Load environment variables from .env file

class ResponseModel(BaseModel):
    topic: str
    summary: str
    source: list[str]
    tools_used: list[str]

llm2 = ChatOpenAI(model="gpt-4o-mini")
llm = ChatAnthropic(model="claude-sonnet-4-6")

parser = PydanticOutputParser(pydantic_object=ResponseModel)

prompt = ChatPromptTemplate.from_messages(
    [
        (
            "system",
            """
            You are a fast translator that needs to translate text from one language to Vietnamese.
            Answer the user query and use neccessary tools. 
            Your response must be strictly only the translation and nothing else\n\n{format_instructions}
            """,
        ),
        ("placeholder", "{chat_history}"),
        ("human", "{query}"),
        ("placeholder", "{agent_scratchpad}"),
    ]
).partial(format_instructions=parser.get_format_instructions())

def show_response(feedback: str):
    popup = tk.Toplevel()
    popup.title("Translation - Vietnamese")

    width = min(max(len(feedback) // 2, 30), 80)
    height = min(max(feedback.count('\n') + 3, 5), 20)
    popup.geometry(f"{width*7}x{height*20}")  # approximate sizing

    text_widget = tk.Text(popup, wrap="word", font=("Arial", 12))
    text_widget.pack(expand=True, fill="both", padx=10, pady=10)

    text_widget.insert("1.0", feedback)
    text_widget.config(state="disabled")

def check_user_input():
    user_query = entry_box1.get("1.0", "end-1c")
    if not user_query.strip():
        error_box.config(text="Please enter text to translate.")
        return
    error_box.config(text="")
    
    formatted_prompt = prompt.format_prompt(query=user_query)
    messages = formatted_prompt.to_messages()
    
    print("----- PROMPT TO LLM -----")
    for m in messages:
        print(f"{m.type.upper()}: {m.content}")
    print("----------------------------")

    llm_response = llm.invoke(messages)

    print(f"----- RAW LLM RESPONSE -----\n{llm_response.content}\n")
    print("-------------------------")

    parsed = parser.parse(llm_response.content)
    print(f"----- PARSED OUTPUT -----\n{parsed}")
    print("-------------------------")
    
    show_response(parsed.summary)

window = tk.Tk()
window.geometry("400x210")
window.title("Translator App")

label1 = tk.Label(window, text="Enter text to translate:", font=("Arial", 12))
label1.place(x=10, y=20, width=380, height=25)

entry_box1 = tk.Text(window, font=("Arial", 12), wrap="word")
entry_box1.place(x=10, y=50, width=380, height=80)

button = tk.Button(text = "Translate", command=check_user_input)
button.config(font = 16)
button.place(x = 10, y = 170, width = 380, height = 25)

error_box = tk.Label(text = "")
error_box.place(x = 10, y = 130)

window.mainloop()
