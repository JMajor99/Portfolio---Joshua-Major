from langchain.tools import BaseTool

from AI_API.main import check_user_input

class MyTranslatorTool(BaseTool):
    name = "Translator"
    description = "Translates text to Vietnamese. Input should be a string of text to translate."

    def _run(self, text: str, target_language: str = "Vietnamese") -> str:
        return f"[Translated to {target_language}]: {text}"

    async def _arun(self, text: str, target_language: str = "Vietnamese") -> str:
        raise NotImplementedError("Async not implemented")   
