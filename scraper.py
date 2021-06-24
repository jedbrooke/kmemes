import json
import requests as req
import os
import subprocess
import time
import sys


DATA_PATH = "/home/jedbrooke/Documents/source/kmemes/memes.json"
IMAGE_DIR = "/home/jedbrooke/Documents/source/kmemes/memes"
DATA_URL = "http://www.reddit.com/r/memes/hot.json"

def load_from_web(data_url):
    request_data = req.get(data_url)
    if request_data.status_code >= 500:
        print(json.loads(request_data.text)["message"],file=sys.stderr)
        sys.exit()
    elif request_data.status_code >= 400:
        print(json.loads(request_data.text)["message"],file=sys.stderr)
        sys.exit()

    json_data = json.loads(request_data.text)
    print(json_data)
    with open(DATA_PATH,"w") as fh:
        fh.write(json.dumps(json_data))

if __name__ == '__main__':
    data_url = DATA_URL
    if len(sys.argv) > 1:
        data_url = sys.argv[1]

    if not os.path.exists(DATA_PATH):
        load_from_web(data_url)
    
    with open(DATA_PATH,"r") as data_file:
        json_data = json.loads(data_file.read())
    
    images = os.listdir(IMAGE_DIR)
    print(images)

    for post in json_data["data"]["children"]:
        if not post["data"]["is_video"]:
            url = post["data"]["url"]
            if url.split("/")[-1] in images:
                print("skipping",url)
            else:
                print("getting",url)
                subprocess.call(["wget","-P",IMAGE_DIR,url],stdout=sys.stdout)
                time.sleep(1)
    os.remove(DATA_PATH)

    
