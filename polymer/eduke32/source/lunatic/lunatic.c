/* The Lunatic Interpreter, part of EDuke32 */

#include <stdint.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "cache1d.h"
#include "osd.h"

#include "gameexec.h"
#include "gamedef.h"  // EventNames[], MAXEVENTS

#include "lunatic.h"
#include "lunatic_priv.h"

// this serves two purposes:
// the values as booleans and the addresses as keys to the Lua registry
uint8_t g_elEvents[MAXEVENTS];

// same thing for actors:
uint8_t g_elActors[MAXTILES];

// Lua-registry key for debug.traceback
static uint8_t debug_traceback_key;

// for timing events and actors
uint32_t g_eventCalls[MAXEVENTS], g_actorCalls[MAXTILES];
double g_eventTotalMs[MAXEVENTS], g_actorTotalMs[MAXTILES];


// forward-decls...
static int32_t SetEvent_luacf(lua_State *L);
static int32_t SetActor_luacf(lua_State *L);

// in lpeg.o
extern int luaopen_lpeg(lua_State *L);


typedef struct {
    uint32_t x, y, z, c;
} rng_jkiss_t;

// See: Good Practice in (Pseudo) Random Number Generation for
//      Bioinformatics Applications, by David Jones
ATTRIBUTE((optimize("O2")))
uint32_t rand_jkiss_u32(rng_jkiss_t *s)
{
    uint64_t t;
    s->x = 314527869 * s->x + 1234567;
    s->y ^= s->y << 5; s->y ^= s->y >> 7; s->y ^= s->y << 22;
    t = 4294584393ULL * s->z + s->c; s->c = t >> 32; s->z = t;
    return s->x + s->y + s->z;
}

ATTRIBUTE((optimize("O2")))
double rand_jkiss_dbl(rng_jkiss_t *s)
{
    double x;
    unsigned int a, b;
    a = rand_jkiss_u32(s) >> 6; /* Upper 26 bits */
    b = rand_jkiss_u32(s) >> 5; /* Upper 27 bits */
    x = (a * 134217728.0 + b) / 9007199254740992.0;
    return x;
}


void El_PrintTimes(void)
{
    int32_t i;

    OSD_Printf("{\n {\n");
    OSD_Printf("  -- event times, [event]={ total calls, total time in ms, time per call in us }\n");
    for (i=0; i<MAXEVENTS; i++)
        if (g_eventCalls[i])
            OSD_Printf("  [%2d]={ %8d, %9.3f, %9.3f },\n",
                       i, g_eventCalls[i], g_eventTotalMs[i],
                       1000*g_eventTotalMs[i]/g_eventCalls[i]);

    OSD_Printf(" },\n\n {\n");
    OSD_Printf("  -- actor times, [tile]={ total calls, total time in ms, time per call in us }\n");
    for (i=0; i<MAXTILES; i++)
        if (g_actorCalls[i])
            OSD_Printf("  [%5d]={ %8d, %9.3f, %9.3f },\n",
                       i, g_actorCalls[i], g_actorTotalMs[i],
                       1000*g_actorTotalMs[i]/g_actorCalls[i]);
    OSD_Printf(" },\n}\n");
}


// 0: success, <0: failure
int32_t El_CreateState(El_State *estate, const char *name)
{
    lua_State *L;

    estate->name = Bstrdup(name);
    if (!estate->name)
        return -1;

    L = estate->L = luaL_newstate();

    if (!estate->L)
    {
        Bfree(estate->name);
        estate->name = NULL;
        return -2;
    }

    luaL_openlibs(L);  // NOTE: we set up the sandbox in defs.ilua
    luaopen_lpeg(L);
    lua_pop(L, lua_gettop(L));  // pop off whatever lpeg leaves on the stack

    setup_debug_traceback(L);

    // create misc. global functions in the Lua state
    lua_pushcfunction(L, SetEvent_luacf);
    lua_setglobal(L, "gameevent");
    lua_pushcfunction(L, SetActor_luacf);
    lua_setglobal(L, "gameactor");

    Bassert(lua_gettop(L)==0);

    return 0;
}

void El_DestroyState(El_State *estate)
{
    if (!estate->L)
        return;

    Bfree(estate->name);
    estate->name = NULL;

    lua_close(estate->L);
    estate->L = NULL;
}

// -1: alloc failure
// 0: success
// 1: didn't find file
// 2: couldn't read whole file
// 3: syntax error in lua file
// 4: runtime error while executing lua file
int32_t El_RunOnce(El_State *estate, const char *fn)
{
    return lunatic_run_once(estate->L, fn, estate->name);
}


////////// Lua_CFunctions //////////

// gameevent(EVENT_..., lua_function)
static int32_t SetEvent_luacf(lua_State *L)
{
    int32_t eventidx;

    if (lua_gettop(L) != 2)
        luaL_error(L, "gameevent: must pass exactly two arguments");

    eventidx = luaL_checkint(L, 1);

    luaL_argcheck(L, (unsigned)eventidx < MAXEVENTS, 1, "must be an event number (0 .. MAXEVENTS-1)");
    check_and_register_function(L, &g_elEvents[eventidx]);
    g_elEvents[eventidx] = 1;

    return 0;
}

// gameactor(<actortile>, lua_function)
static int32_t SetActor_luacf(lua_State *L)
{
    int32_t actortile;

    if (lua_gettop(L) != 2)
        luaL_error(L, "gameactor: must pass exactly two arguments");

    actortile = luaL_checkint(L, 1);

    luaL_argcheck(L, (unsigned)actortile < MAXTILES, 1, "must be an tile number (0 .. MAXTILES-1)");
    check_and_register_function(L, &g_elActors[actortile]);
    g_elActors[actortile] = 1;

    return 0;
}

//////////////////////////////

static int32_t call_registered_function3(lua_State *L, void *keyaddr,
                                         int32_t iActor, int32_t iPlayer, int32_t lDist)
{
    int32_t i;

    // get the Lua function from the registry
    lua_pushlightuserdata(L, keyaddr);
    lua_gettable(L, LUA_REGISTRYINDEX);

    lua_pushinteger(L, iActor);
    lua_pushinteger(L, iPlayer);
    lua_pushinteger(L, lDist);

    // -- call it! --

    i = lua_pcall(L, 3, 0, 0);
    if (i == LUA_ERRMEM)
    {
        lua_pop(L, 1);
        // XXX: should be more sophisticated.  Clean up stack? Do GC?
    }

    return i;
}

int32_t El_CallEvent(El_State *estate, int32_t eventidx, int32_t iActor, int32_t iPlayer, int32_t lDist)
{
    // XXX: estate must be the one where the events were registered...
    //      make a global?

    lua_State *const L = estate->L;

    int32_t i = call_registered_function3(L, &g_elEvents[eventidx], iActor, iPlayer, lDist);

    if (i == LUA_ERRRUN)
    {
        OSD_Printf("event \"%s\" (state \"%s\") runtime error: %s\n", EventNames[eventidx].text,
                   estate->name, lua_tostring(L, -1));  // get err msg
        lua_pop(L, 1);
        return 4;
    }

    return 0;
}

int32_t El_CallActor(El_State *estate, int32_t actortile, int32_t iActor, int32_t iPlayer, int32_t lDist)
{
    lua_State *const L = estate->L;

    int32_t i = call_registered_function3(L, &g_elActors[actortile], iActor, iPlayer, lDist);

    if (i == LUA_ERRRUN)
    {
        OSD_Printf("actor %d (sprite %d, state \"%s\") runtime error: %s\n", actortile, iActor,
                   estate->name, lua_tostring(L, -1));  // get err msg
        lua_pop(L, 1);
        return 4;
    }

    return 0;
}
