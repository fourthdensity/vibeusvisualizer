import os
import json
import requests

api_key = os.getenv("XAI_API_KEY")
if not api_key:
    raise ValueError("Set XAI_API_KEY environment variable first")

url = "https://api.x.ai/v1/chat/completions"
headers = {
    "Authorization": f"Bearer {api_key}",
    "Content-Type": "application/json"
}
payload = {
    "model": "grok-4",
    "messages": [
        {"role": "user", "content": "Hello Grok, what can you do?"}
    ],
    "temperature": 0.7
}

response = requests.post(url, headers=headers, json=payload)
if response.status_code != 200:
    print("Error:", response.status_code, response.text)
    exit(1)

data = response.json()
print(data["choices"][0]["message"]["content"])