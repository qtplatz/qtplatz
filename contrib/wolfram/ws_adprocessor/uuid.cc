// https://gist.github.com/wichert/5543390
#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <adportable/debug.hpp>

namespace {

    boost::uuids::nil_generator nil_gen;
    boost::uuids::string_generator parse_uuid;

        uuid_from_python() {
            //ADDEBUG() << "***** uuid_from_python ctor *****";
            python::converter::registry::push_back(&convertible, &construct, python::type_id<boost::uuids::uuid>());
            //ADDEBUG() << "***** uuid_from_python registered.";
        }

        static void * convertible( PyObject *obj ) {
            //ADDEBUG() << "***** " << __FUNCTION__;
            if (obj == Py_None || PyUnicode_Check(obj) /*|| PyString_Check(obj)*/ )
                return obj;
            return 0;
        }

        static void construct(PyObject* obj, python::converter::rvalue_from_python_stage1_data *data) {
            typedef python::converter::rvalue_from_python_storage<boost::uuids::uuid> storage_type;

            //ADDEBUG() << "***** " << __FUNCTION__;
            boost::uuids::uuid uuid{{0}};
            if (obj==Py_None)
                uuid=nil_gen();
            else {
                PyObject *str = 0;
                const char* value = "";

                if (PyUnicode_Check(obj)) {
                    str = PyUnicode_AsUTF8String(obj);
                    if ( str == 0 )
                        python::throw_error_already_set();
                    obj = str;
                }
                value = PyUnicode_AsUTF8( obj ); //value=PyString_AsString(obj);
                Py_XDECREF(str);
                if (value==0)
                    python::throw_error_already_set();
                try {
                    uuid = parse_uuid(value);
                } catch (const std::runtime_error &e) {
                    PyErr_SetString(PyExc_ValueError, "Invalid UUID value.");
                    python::throw_error_already_set();
                }
            }

            void* storage = reinterpret_cast<storage_type*>(data)->storage.bytes;
            new (storage) boost::uuids::uuid(uuid);
            data->convertible = storage;
        }
    };
}

void
exportUUID()
{
    boost::python::to_python_converter< boost::uuids::uuid, uuid_to_python >();
    uuid_from_python();
}
