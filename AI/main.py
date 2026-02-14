from dotenv import load_dotenv
from pydantic import BaseModel
from langchain_openai import ChatOpenAI
from langchain_anthropic import ChatAnthropic

load_dotenv() # Load environment variables from .env file

llm = ChatOpenAI()
llm2 = ChatAnthropic()