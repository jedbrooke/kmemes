import json
import requests
import requests.auth
import sys
import os
import time
import shutil


DATA_PATH = "/home/jedbrooke/source/kmemes/memes.json"
IMAGE_DIR = "/home/jedbrooke/source/kmemes/memes"

SUBREDDITS = ["memes","dankmemes","pics","itookapicture","photoshopbattles","wtfstockphotos","blender"]
AUTH_PATH = "auth.json"

def load_from_web(data_url):
    request_data = requests.get(data_url)
    if request_data.status_code != 200:
        print(json.loads(request_data.text)["message"],file=sys.stderr)
        sys.exit()

    json_data = json.loads(request_data.text)
    with open(DATA_PATH,"w") as fh:
        fh.write(json.dumps(json_data))

if __name__ == '__main__':
    with open(AUTH_PATH) as fh:
        auth_data = json.load(fh)
    client_auth = requests.auth.HTTPBasicAuth(auth_data["client-ID"],auth_data["client-secret"])
    post_data = {"grant_type": "password", "username": auth_data["username"], "password": auth_data["password"]}
    headers = {"User-Agent": f"kmemes_bot/0.1 by {auth_data['username']}"}
    auth_response = requests.post("https://www.reddit.com/api/v1/access_token", auth=client_auth, data=post_data, headers=headers)
    if auth_response.status_code != 200:
        print(auth_response.headers,file=sys.stderr)
        print(auth_response.text,file=sys.stderr)
        exit()
    response = json.loads(auth_response.text)
    token = response["access_token"]
    token_type = response["token_type"]

    headers["Authorization"] = f"{token_type} {token}"
    for sub in SUBREDDITS:
        print("scanning sub",sub)
        response = requests.get(f"https://oauth.reddit.com/r/{sub}/hot",headers=headers)
        json_data = json.loads(response.text)
        for post in json_data["data"]["children"]:
            if not post["data"]["is_video"]:
                url = post["data"]["url"]
                path = os.path.join(IMAGE_DIR,os.path.basename(url))
                if not os.path.exists(path):
                    response = requests.get(url, stream=True)
                    print("getting",url)
                    if response.status_code == 200:
                        with open(os.path.join(IMAGE_DIR,os.path.basename(url)),"wb") as fh:
                            shutil.copyfileobj(response.raw,fh)
                            # sleep for 1 second to be nicer on bandwidth
                            time.sleep(1)


    
