#ifndef _PicoScope_MODULE_H
#define _PicoScope_MODULE_H

#include <libPicoScope.h>
#include <unistd.h>

static PyObject * open_unit( PyObject *self, PyObject *args );
static PyObject * close_unit( PyObject *self, PyObject *args );
//static PyObject * get_info( PyObject *self, PyObject *args );
//static PyObject * flash_led( PyObject *self, PyObject *args );
static PyObject * set_channel( PyObject *self, PyObject *args );
//static PyObject * get_timebase( PyObject *self, PyObject *args );
static PyObject * set_trigger( PyObject *self, PyObject *args );
static PyObject * ready( PyObject *self, PyObject *args );
//static PyObject * stop( PyObject *self, PyObject *args );
//static PyObject * set_ets( PyObject *self, PyObject *args );
//static PyObject * set_led( PyObject *self, PyObject *args );
//static PyObject * set_light( PyObject *self, PyObject *args );
static PyObject * delay( PyObject *self, PyObject *args );
static PyObject * run_block( PyObject *self, PyObject *args );
static PyObject * get_values( PyObject *self, PyObject *args );

PyMODINIT_FUNC initPicoScope( void );

#endif
