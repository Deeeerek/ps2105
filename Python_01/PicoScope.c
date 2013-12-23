#include <Python.h>
#include "PicoScope.h"

//#define DEBUG

// This table defines all the functions in the module
static PyMethodDef PicoScope_Methods[] = {
    {"open_unit",    open_unit,     METH_VARARGS, "Open unit"                     },
    {"close_unit",   close_unit,    METH_VARARGS, "Close unit"                    },
    {"ready",        ready,         METH_VARARGS, "Oscilloscope ready"            },
    {"set_channel",  set_channel,   METH_VARARGS, "Set oscilloscope channel"      },
    {"set_trigger",  set_trigger,   METH_VARARGS, "Set oscilloscope trigger level"},
    {"run_block",    run_block,     METH_VARARGS, "Run block"                     },
    {"get_values",   get_values,    METH_VARARGS, "Get values"                    },
    {"delay",        delay,         METH_VARARGS, "Delay in ms"                   },
    { NULL,          NULL,          0,            NULL                            } // marker for end-of-table
};

// Error variable
static PyObject *PicoScope_Error;

static PyObject *
open_unit( PyObject *self, PyObject *args ){
    struct PicoScopeDev_t *PicoDev;

    PicoDev = malloc( sizeof(struct PicoScopeDev_t) );

    ps2000_open_unit(PicoDev);

    return Py_BuildValue("l", PicoDev);
}


static PyObject *
close_unit( PyObject *self, PyObject *args ){
    struct PicoScopeDev_t *PicoDev;
    int returnCode = 0;

    if( !PyArg_ParseTuple(args, "l", &PicoDev) )
        return NULL;

    returnCode = ps2000_close_unit(PicoDev);

    free(PicoDev);

    return Py_BuildValue("i", returnCode);
}


static PyObject *
ready( PyObject *self, PyObject *args ){
    struct PicoScopeDev_t *PicoDev;
    int                   returnCode;

    if( !PyArg_ParseTuple(args, "l", &PicoDev) )
        return NULL;
    
    returnCode = ps2000_ready(PicoDev);

    return Py_BuildValue("i", returnCode);
}


//
//    returnCode = set_channel( handle, channel, enabled, dc, range );
//
static PyObject *
set_channel( PyObject *self, PyObject *args ){
    struct PicoScopeDev_t *PicoDev;
    int                   channel;
    int                   enabled;
    int                   dc;
    int                   range;
    int       returnCode;

    if( !PyArg_ParseTuple(args, "liiii", &PicoDev, &channel, &enabled, &dc, &range) )
        return NULL;

#ifdef DEBUG
    printf("PicoScope.set_channel( %d, %d );\n", dc, range );
#endif
    
    returnCode = ps2000_set_channel( PicoDev, dc, range );

    return Py_BuildValue("i", returnCode);
}


static PyObject *
set_trigger( PyObject *self, PyObject *args ){
    struct PicoScopeDev_t *PicoDev;
    int                   source;
    int                   threshold;
    int                   direction;
    int                   delay;
    int                   autoTrigger;
    int                   returnCode;

    if( !PyArg_ParseTuple(args, "liiiii", &PicoDev, &source, &threshold, &direction, &delay, &autoTrigger) )
        return NULL;
    
#ifdef DEBUG
    printf("PicoScope.set_trigger( %d, %d, %d );\n",threshold,direction,delay);
#endif

    returnCode = ps2000_set_trigger( PicoDev, 0.0, direction, delay );

    return Py_BuildValue("i", returnCode);
}


static PyObject *
run_block( PyObject *self, PyObject *args ){
    struct PicoScopeDev_t *PicoDev;
    long                  numberOfSamples;
    int                   timebase;
    int                   oversample;
    long                  timeIndisposed_ms = 0;
    int                   returnCode;

    if( !PyArg_ParseTuple(args, "llii", &PicoDev, &numberOfSamples, &timebase, &oversample) )
        return NULL;

#ifdef DEBUG
    printf("PicoScope.run_block( %d, %ld );\n", timebase, numberOfSamples );
#endif

    returnCode = ps2000_run_block( PicoDev, timebase, numberOfSamples );

    return Py_BuildValue("{s:i,s:i}", "returnCode", returnCode,
                                      "timeIndisposed_ms", timeIndisposed_ms);
}


static PyObject *
get_values( PyObject *self, PyObject *args ){
    struct PicoScopeDev_t *PicoDev;
    long                  numberOfValues;
    short                 *samplesChannelA;
    long                  ii;
    PyObject              *p;
    PyObject              *item;

    if( !PyArg_ParseTuple(args, "ll", &PicoDev, &numberOfValues ) )
        return NULL;

#ifdef DEBUG
    printf("PicoScope.get_values( %ld );\n", numberOfValues);
#endif

    samplesChannelA = (short *)malloc(sizeof(short)*numberOfValues);
    ps2000_get_values( PicoDev,
                       samplesChannelA,   // channel A
                       numberOfValues);
#if 0
//#ifdef DEBUG
    for (int ii=0; ii<numberOfValues; ii++ ){
//        printf("%04x ", samplesChannelA[ii] & 0xffff );
        printf("%-5d  ", samplesChannelA[ii] );
        if ( (ii%16)==15 ) printf("\n");
    }
    printf("\n");
//#endif
#endif
    p = PyList_New(numberOfValues);
    for( ii=0; ii<numberOfValues; ii++ ){
        item = PyInt_FromLong((int)samplesChannelA[ii]);
        PyList_SetItem(p,ii,item);
    }
    free(samplesChannelA);
    return p;
}


static PyObject *
delay( PyObject *self, PyObject *args ){
    const int  value;

    if( !PyArg_ParseTuple(args, "i", &value) )
        return NULL;

    usleep(1000*value);

    Py_RETURN_NONE;
}









PyMODINIT_FUNC
initPicoScope( void )
{
    PyObject *m;

    m = Py_InitModule("PicoScope", PicoScope_Methods);
    if (m == NULL)
        return;

    PicoScope_Error = PyErr_NewException("PicoScope.error", NULL, NULL);
    Py_INCREF(PicoScope_Error);
    PyModule_AddObject(m, "error", PicoScope_Error);
}



int main(int argc, char *argv[])
{
    printf("/* Pass argv[0] to the Python interpreter */\n");
    fflush(stdout);
    Py_SetProgramName(argv[0]);

    printf("/* Initialize the Python interpreter.  Required. */\n");
    fflush(stdout);
    Py_Initialize();

    printf("/* Add a static module */\n");
    fflush(stdout);
    initPicoScope();

    return 0;
}


