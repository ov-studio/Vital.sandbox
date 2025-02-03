src: "https://github.com/lua/lua/releases"
version: "v5.4.4"
externals:
    -: "https://github.com/starwing/luautf8/blob/master/unidata.h"
    -: "https://github.com/starwing/luautf8/blob/master/lutf8lib.c"
dependencies:
    luautf8:
        src: "https://github.com/starwing/luautf8/releases"
        version: "v0.1.6"
    rapidjson:
        src: "https://github.com/xpol/lua-rapidjson/releases"
        version: "v0.7.1"