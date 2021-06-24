import json
import requests as req
import os
import subprocess
import time


DATA_PATH = "memes.json"
IMAGE_DIR = "memes"

def load_from_web():
    request_data = req.get("http://www.reddit.com/r/memes/hot.json")
    json_data = json.loads(request_data.text)
    with open(DATA_PATH,"w") as fh:
        fh.write(json.dumps(json_data))

if __name__ == '__main__':
    if not os.path.exists(DATA_PATH):
        load_from_web()
    with open(DATA_PATH,"r") as data_file:
        json_data = json.loads(data_file.read())
    for post in json_data["data"]["children"]:
        if not post["data"]["is_video"]:
            url = post["data"]["url"]
            print("getting",url)
            subprocess.call(["wget","-P",IMAGE_DIR,url])
            time.sleep(1)
    
