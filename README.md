# Current Plan

- `GET /` serves single-page app
- From there, clients `POST /join` with a username and are assigned a UUID
    - The UUID is stored in local storage and is used to authenticate the user for future requests
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
            "uuid":"XXXX",
            "admin": true
          }
          ```
    - /lobby
        - ```js
          // GET /lobby response
          {
            "admin": true,
            "taken_pfps": [0, 1, 5, 8],
            "timeout": 5
          }

          // POST /lobby request
          {
            "uuid":"XXXX",
            "pfp":10,
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
          }

          // POST /draw request
          {
            "uuid":"XXXX",
            "image": "some kind of raw image data here",
          }
          ```
    - /write
        - ```js
          // GET /write response
          {
            "round": 1
            "timeout": 90
          }

          // POST /write request
          {
            "uuid":"XXXX",
            "slogan": "words go here"
          }
          ```
    - /shirt
        - ```js
          // GET /shirt response
          {
            "round": 1
            "images": ["image data", "image data", "image data"],
            "image_indices": [7, 29, 3],
            "slogans": [text, text, text],
            "slogan_indices": [32, 8, 21],
            "timeout": 90
          }

          // POST /shirt request
          {
            "uuid":"XXXX",
            "image": 0,
            "slogan": 2,
            "round": 1
          }
          ```
    - /battle
        - ```js
            // GET /battle response
            {
            "round": 1
            "timeout": 90
            "slogans": ["shirt 0", "shirt 1"]
            }

            // POST /battle request
            {
            "uuid":"XXXX",
            "vote": 0,
            "round": 1
            }
            ```


- There will be three threads: game state, graphics, network
    - Game state is encapsulated in list of users, images, slogans, shirts
        - ```c
            enum gamestate {
                LOBBY
                STARTING
                DRAW
                WAIT_AFTER_DRAW
                SLOGANS
                WAIT_AFTER_SLOGANS
                SHIRT
                WAIT_AFTER_SHIRT
                BATTLE
            }

            #define MAX_PLAYERS 8
            #define MAX_IMAGES MAX_PLAYERS * 3
            #define MAX_SLOGANS MAX_PLAYERS * 8

            struct game_data {
                gamestate state
                player_t players[MAX_PLAYERS];
                image_t images[MAX_IMAGES];
                slogan_t slogans[MAX_PLAYERS];
            }

            struct player {
                uint32_t uuid;
                char* username;
                char* victory_quote;
                uint8_t pfp;
                bool admin;
            }

            struct image {
                uint32_t creator_uuid;
                char* image;
            }

            struct slogan {
                uint32_t creator_uuid;
                char* image;
            }
        ```



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