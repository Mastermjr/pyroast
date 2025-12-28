#include <Python.h>
#include <string.h>

static int base32hex_lookup[256];

static void init_lookup(void) {
    memset(base32hex_lookup, -1, sizeof(base32hex_lookup));
    for (int i = 0; i < 10; i++) {
        base32hex_lookup['0' + i] = i;
    }
    for (int i = 0; i < 22; i++) {  // a-v / A-V
        unsigned char lower = 'a' + i;
        unsigned char upper = 'A' + i;
        base32hex_lookup[lower] = 10 + i;
        base32hex_lookup[upper] = 10 + i;
    }
}

static PyObject *decode_oast_method(PyObject *self, PyObject *args) {
    PyObject *input_obj;
    if (!PyArg_ParseTuple(args, "O", &input_obj)) return NULL;

    PyObject *seq = NULL;
    if (PyUnicode_Check(input_obj)) {
        seq = PyList_New(1);
        if (!seq) return NULL;
        Py_INCREF(input_obj);
        PyList_SET_ITEM(seq, 0, input_obj);
    } else {
        seq = PySequence_List(input_obj);
        if (!seq) {
            PyErr_SetString(PyExc_TypeError, "Expected str or iterable of str");
            return NULL;
        }
    }

    Py_ssize_t n = PyList_Size(seq);
    PyObject *originals = PyList_New(n);
    PyObject *timestamps = PyList_New(n);
    PyObject *machines = PyList_New(n);
    PyObject *pids = PyList_New(n);
    PyObject *counters = PyList_New(n);
    if (!originals || !timestamps || !machines || !pids || !counters) {
        Py_XDECREF(seq); Py_XDECREF(originals); Py_XDECREF(timestamps);
        Py_XDECREF(machines); Py_XDECREF(pids); Py_XDECREF(counters);
        return PyErr_NoMemory();
    }

    for (Py_ssize_t i = 0; i < n; i++) {
        PyObject *item = PyList_GET_ITEM(seq, i);
        Py_INCREF(item);
        PyList_SET_ITEM(originals, i, item);

        const char *sub = PyUnicode_AsUTF8(item);
        PyObject *none = Py_None;
        int valid = (sub && strlen(sub) >= 20);

        uint8_t bytes[12] = {0};
        if (valid) {
            uint64_t buffer = 0;
            int bits = 0;
            int byte_idx = 0;
            const char *p = sub;
            int j;
            for (j = 0; j < 20; j++) {
                int val = base32hex_lookup[(unsigned char)p[j]];
                if (val < 0) { valid = 0; break; }
                buffer = (buffer << 5) | val;
                bits += 5;
                while (bits >= 8 && byte_idx < 12) {
                    bits -= 8;
                    bytes[byte_idx++] = (buffer >> bits) & 0xFF;
                }
            }
            if (j != 20 || byte_idx != 12) valid = 0;
        }

        if (valid) {
            uint32_t ts = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
            PyList_SET_ITEM(timestamps, i, PyLong_FromUnsignedLong(ts));

            char mach[9];
            PyOS_snprintf(mach, 9, "%02x:%02x:%02x", bytes[4], bytes[5], bytes[6]);
            PyList_SET_ITEM(machines, i, PyUnicode_FromString(mach));

            uint32_t pid_val = (bytes[7] << 8) | bytes[8];
            PyList_SET_ITEM(pids, i, PyLong_FromUnsignedLong(pid_val));

            uint32_t cnt = (bytes[9] << 16) | (bytes[10] << 8) | bytes[11];
            PyList_SET_ITEM(counters, i, PyLong_FromUnsignedLong(cnt));
        } else {
            Py_INCREF(none); PyList_SET_ITEM(timestamps, i, none);
            Py_INCREF(none); PyList_SET_ITEM(machines, i, none);
            Py_INCREF(none); PyList_SET_ITEM(pids, i, none);
            Py_INCREF(none); PyList_SET_ITEM(counters, i, none);
        }
    }

    Py_DECREF(seq);
    PyObject *result = PyTuple_Pack(5, originals, timestamps, machines, pids, counters);
    Py_DECREF(originals); Py_DECREF(timestamps); Py_DECREF(machines);
    Py_DECREF(pids); Py_DECREF(counters);
    return result;
}

static PyMethodDef methods[] = {
    {"_decode_oast", decode_oast_method, METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "_core",
    NULL,
    -1,
    methods
};

PyMODINIT_FUNC PyInit__core(void) {
    PyObject *m = PyModule_Create(&moduledef);
    if (!m) return NULL;
    init_lookup();
    return m;
}