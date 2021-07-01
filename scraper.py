import json
import requests as req
import os
import subprocess
import time
import sys



DATA_PATH = "/home/jedbrooke/source/kmemes/memes.json"
IMAGE_DIR = "/home/jedbrooke/source/kmemes/memes"
SUBREDDITS = ["memes","dankmemes","pics","itookapicture","photoshopbattles","wtfstockphotos"]
AUTH_PATH = "auth.json"

def load_from_web(data_url):
    request_data = req.get(data_url)
    if request_data.status_code != 200:
        print(json.loads(request_data.text)["message"],file=sys.stderr)
        sys.exit()

    json_data = json.loads(request_data.text)
    with open(DATA_PATH,"w") as fh:
        fh.write(json.dumps(json_data))

if __name__ == '__main__':
    with open(AUTH_PATH) as fh:
        auth_data = json.load(fh)
    client_auth = req.auth.HTTPBasicAuth(auth_data["client-ID"],auth_data["client-secret"])
    post_data = {"grant_type": "password", "username": auth_data["username"], "password": auth_data["password"]}
    headers = {"User-Agest": "Kmemes_bot/0.1 by jedbrooke"}
    auth_response = req.post("https://www.reddit.com/api/v1/acces_token", auth=client_auth, data=post_data, headers=headers)
    print(auth_response.status_code)
    print(auth_response.headers)
    print(auth_response.text)

    
