import json
import requests as req
import os
import subprocess
import time
import sys


DATA_PATH = "/home/jedbrooke/source/kmemes/memes.json"
IMAGE_DIR = "/home/jedbrooke/source/kmemes/memes"
DATA_URL = "http://www.reddit.com/r/memes/hot.json"

def load_from_web(data_url):
    request_data = req.get(data_url)
    if request_data.status_code != 200:
        print(json.loads(request_data.text)["message"],file=sys.stderr)
        sys.exit()

    json_data = json.loads(request_data.text)
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
    count = 0

    for post in json_data["data"]["children"]:
        if not post["data"]["is_video"]:
            url = post["data"]["url"]
            ext = url.split(".")[-1]
            if url.split("/")[-1] in images or ext == "gifv":
                print("skipping",url)
            else:
                count += 1
                print("getting",url)
                subprocess.call(["wget","-P",IMAGE_DIR,url],stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                time.sleep(10)
    print("fetched",count,"memes")
    os.remove(DATA_PATH)

    
