# Current Plan

- `GET /` serves single-page app
- From there, clients `POST /join` with a username and are assigned a id
    - The id is stored in local storage and is used to authenticate the user for future requests
- Clients continuously poll `GET /state` to get the gamestate and show the correct screen
- Clients poll `GET /[gamestate_goes_here]` at the start of every state change.
    - /join
        - ```js
          // POST /join request
          {
            "username":"potato",
          }

          // POST /join response
          {
            "id":"XXXX",
          }
          ```
    - /lobby
        - ```js
          // GET /lobby response
          {
            "admin": true,
          }

          // POST /lobby request
          {
            "id":"XXXX",
            "pfp":10,
            (or)
            "quote":"quote goes here"
          }
          ```
    - wait
    - /draw
        - ```js
          // GET /draw response
          {
            "round": 1
            "timeout": 90
            "submitted": false
          }

          // POST /draw request
          {
            "id":"XXXX",
            "image": "terrible jpeg data url goes here",
          }
          ```
    - /write
        - ```js
          // GET /write response
          {
            "timeout": 90
          }

          // POST /write request
          {
            "id":"XXXX",
            "slogan": "words go here"
          }
          ```
    - /shirt
        - ```js
          // GET /shirt response
          {
            "round": 1
            "images": ["image path", "image path", "image path"],
            "slogans": ["text", "text", "text"],
            "timeout": 90
          }

          // POST /shirt request
          {
            "id":"XXXX",
            "image": "image path",
            "slogan": "text",
          }
          ```
    - /battle
        - ```js
            // GET /battle response
            {
            "timeout": 90
            "slogans": ["shirt 0", "shirt 1"]
            }

            // POST /battle request
            {
            "id":"XXXX",
            "vote": 0,
            }
            ```


- There will be two threads: graphics and network
  - Game state will be shared between the two threads in an object that implements the gameplay state machine.

Routing
- Any of the API routes that respond to JSON are checked for explicitly
- GET / returns index.html
- Any other request is attempted to be found on the filesystem under data/wiiko/web/ or temp/wiiko
- TODO: encode id for get requests as a query parameter? as an Authentication: Bearer Token header?


Files
- data/wiiko/web/ will store the sveltekit website. Ideally this would get compacted into one site but idk. Maybe if we can translate the dynamic imports to an older form of Javascript?
- data/wiiko/settings.json will hold the game settings and default round settings.
- the temp/wiiko folder will be wiped clean when every new game starts.
- temp/wiiko/images will hold user-submitted images. Each image will be stored as `userid/NUM.jpg`
- temp/wiiko/gamestate.json stores current gamestate so that it can be restored at a later time?
- Do we actually want a temp folder though? All of this could just be stored in memory if we don't care that it persists. It depends on how fast the filesystem is, I guess
  - The SD card may be more scalable and allow larger image sizes since we don't have to worry about being able to fit 24+ images in RAM. Either way we'll have to do static serving from there.


## Miis for pfps
- [Jesus](https://www.miicharacters.com/wii.php?mii=1303)
- [Whale](https://www.miicharacters.com/wii.php?mii=1053)
- [Horse](https://www.miicharacters.com/wii.php?mii=1742)
- [Two Face](https://www.miicharacters.com/wii.php?mii=1031)
- [Pumpkin](https://www.miicharacters.com/wii.php?mii=1441)
  - [Alternate Pumpkin](https://www.miicharacters.com/wii.php?mii=1733)
- [Dino](https://www.miicharacters.com/wii.php?mii=1343)
- [Hamburger](https://www.miicharacters.com/wii.php?mii=1313)
- [Shark](https://www.miicharacters.com/wii.php?mii=1959)
- [Mona Lisa](https://www.miicharacters.com/wii.php?mii=1190)
- [Panda](https://www.miicharacters.com/wii.php?mii=1245)
- [Fish](https://www.miicharacters.com/wii.php?mii=1144)
- [Frog](https://www.miicharacters.com/wii.php?mii=1191)
- [Butterfly](https://www.miicharacters.com/wii.php?mii=1270)
- [Basketball](https://www.miicharacters.com/wii.php?mii=1315)
- [Owl](https://www.miicharacters.com/wii.php?mii=1192)
- [Flying Spaghetti Monster](https://www.miicharacters.com/wii.php?mii=817)
- [Turkey](https://www.miicharacters.com/wii.php?mii=1151)
- [Dragon](https://www.miicharacters.com/wii.php?mii=1430)
- [Upside Down](https://www.miicharacters.com/wii.php?mii=50)
- [Butterfly](https://www.miicharacters.com/wii.php?mii=1270)


## HTTP Request
```
GETINFO /state HTTP/1.1
Host: localhost
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:109.0) Gecko/20100101 Firefox/114.0
Accept: */*
Accept-Language: en-US,en;q=0.5
Accept-Encoding: gzip, deflate, br
Referer: http://localhost:5173/
Content-Type: text/plain;charset=UTF-8
Content-Length: 17
Origin: http://localhost:5173
DNT: 1
Connection: keep-alive
Sec-Fetch-Dest: empty
Sec-Fetch-Mode: cors
Sec-Fetch-Site: same-site
Pragma: no-cache
Cache-Control: no-cache
```
```
POST /draw HTTP/1.1
Host: localhost
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:109.0) Gecko/20100101 Firefox/114.0
Accept: */*
Accept-Language: en-US,en;q=0.5
Accept-Encoding: gzip, deflate, br
Referer: http://localhost:5173/
Content-Type: image/jpeg
Content-Length: 4088
Origin: http://localhost:5173
DNT: 1
Connection: keep-alive
Sec-Fetch-Dest: empty
Sec-Fetch-Mode: cors
Sec-Fetch-Site: same-site
Pragma: no-cache
Cache-Control: no-cache
```