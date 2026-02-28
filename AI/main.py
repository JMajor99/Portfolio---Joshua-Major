from pathlib import Path
from dotenv import load_dotenv
from pydantic import BaseModel
from langchain_openai import ChatOpenAI
from langchain_anthropic import ChatAnthropic
from langchain_core.prompts import ChatPromptTemplate
from langchain_core.output_parsers import PydanticOutputParser
import os

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
            You are a fast translator that needs to translate text from one language to another.
            Answer the user query and use neccessary tools. 
            Wrap the output in this format and provide no other text\n{format_instructions}
            """,
        ),
        ("placeholder", "{chat_history}"),
        ("human", "{query}"),
        ("placeholder", "{agent_scratchpad}"),
    ]
).partial(format_instructions=parser.get_format_instructions())

#tools = [search_tool, wiki_tool, save_tool]

user_query = "What is the translation of 'Hello World' in Vietnamese?"
llm_response = llm.invoke(prompt.format_prompt(query=user_query).to_messages())

parsed = parser.parse(llm_response.content)
print(parsed)

# response = llm.invoke("What is the meaning of life?")
# print(response.content)