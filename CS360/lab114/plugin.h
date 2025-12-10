#pragma once

typedef enum {
    PR_SUCCESS,
    PR_FAILED,
    PR_STOP
} PluginResult;

typedef void (*INIT)(void);
typedef PluginResult (*SETTING)(const char name[], const char value[]);
typedef PluginResult (*TRANSFORM)(char *data, int *size);

// Plugin functions
void init(void);
PluginResult setting(const char name[], const char value[]);
PluginResult transform(char *data, int *size);
