// https://gist.github.com/yslking/ed01350c26a2ff0b295f93324e28a26f

/**
 *  For Boost.Python
 * 
 *  Automatically convert between Python list, dict, tuple and C++ STL std::vector, std::map, std::tuple, respectively.
 *  --------------------------------------
 *  自动转换 Python 和 STL 的几个常用数据类型。
 * 
 *  I am familiar with neither metaprogramming nor Boost.Python. This header works but I believe
 *  there are better ways (or even existing libraries) to do these conversions. Please comment at
 *  this page to help me improve it.
 * ---------------------------------------
 *  我干啥啥不行，多教教我。
 *  
 *  std::vector <--> list (or other `seq`)
 *  std::tuple  <--> tuple
 *  std::map    <--> dict
 *  nested / 嵌套
 * 
 *  C++ 11 is required to use this header.
 *  C++ 17 is required to run the following example
 * 
 *  Python:
    import example

    print(example.foo())
    # {'key1': [('ele1', 'ele2', 2.4), ('ele4', 'ele6', 4.8)], 'key2': [('ala1', 'ala2', 8.4), ('ala4', 'ala6', 8.8)]}

    example.bar({
        'pykey1': [
            ('val1', 'val2', 3.44),
            ('val3', 'val4', 4.55),
        ],
        'pykey2': [
            ('val5', 'val7', 0.44),
            ('val6', 'val8', 9.55),
        ]
    })
    # pykey1:
    #    { val1, val2, 3.44 }
    #    { val3, val4, 4.55 }
    #  pykey2:
    #    { val5, val7, 0.44 }
    #    { val6, val8, 9.55 }

 *  C++:

    #include <boost/python.hpp>
    #include <boost/python/module.hpp>
    #include <iostream>
    #include <vector>
    #include <map>
    #include <tuple>
    #include "converter.h"

    using namespace std;


    map<string, vector<std::tuple<string, string, double>>> foo(){
        return {
            {"key1", {
                {"ele1", "ele2", 2.4},
                {"ele4", "ele6", 4.8},
            }},
            {"key2", {
                {"ala1", "ala2", 8.4},
                {"ala4", "ala6", 8.8},
            }},
        };
    }

    void bar(map<string, vector<std::tuple<string, string, double>>> m){
        for(auto [k, v]: m){
            cout << k << ":" << endl;
            for(auto [a, b, c]: v){
                cout << "  { " << a << ", " << b << ", " << c << " }" << endl;
            }
        }
    }

    BOOST_PYTHON_MODULE(example)) {
        using namespace boost::python;
        
        def("foo", foo);
        def("bar", bar);

        // Register converter for std::tuple<Type1[, Type2, Type3, ...]>:
        //    tuple_tuple_converter<Type1[, Type2, Type3, ...]>::register_bidirectional_converter();
        tuple_tuple_converter<string, string, double>::register_bidirectional_converter();

        // Register converter for std::vector<T>
        //    list_vector_converter<T>::register_bidirectional_converter();
        list_vector_converter<std::tuple<string, string, double>>::register_bidirectional_converter();

        // Register converter for std::map<K, V>
        //    dict_map_converter<K, V>::register_bidirectional_converter();
        dict_map_converter<string, vector<std::tuple<string, string, double>>>::register_bidirectional_converter();    
    }


 *    
 *
 **/

#ifndef PY_CONVERTER_HPP
#define PY_CONVERTER_HPP

#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <vector>
#include <map>

using namespace boost::python;

template <typename EleT>
struct list_vector_converter {
    static void register_from_python_converter(){
        converter::registry::push_back(
            &list_vector_converter::convertible,
            &list_vector_converter::construct,
            boost::python::type_id<std::vector<EleT>>());
    }

    static void register_to_python_converter(){
        boost::python::to_python_converter<std::vector<EleT>, list_vector_converter<EleT>, true>();
    }

    static void register_bidirectional_converter(){
        register_from_python_converter();
        register_to_python_converter();
    }

    static void* convertible(PyObject *obj) {
        if(!PySequence_Check(obj))
            return nullptr;
        
        int len = PySequence_Length(obj);
        if(len < 0) return nullptr;
        for(size_t i = 0; i < len; i++) {
            extract<EleT> e(PySequence_GetItem(obj, i));
            if(!e.check()){
                return nullptr;
            }
        }
        return obj;
    }

    static void construct(PyObject *obj,
                          boost::python::converter::rvalue_from_python_stage1_data *data)
    {
        int len = PySequence_Length(obj);
        assert(len >= 0);
        void *storage = ((converter::rvalue_from_python_storage<std::vector<EleT>> *)data)->storage.bytes;
        auto pvec = new (storage) std::vector<EleT>(len);
        for(size_t i = 0; i < len; i++) {
            (*pvec)[i] = extract<EleT>(PySequence_GetItem(obj, i));
        }
        data->convertible = storage;
    }

    static PyObject* convert(const std::vector<EleT>& vec) {
        PyObject* obj = PyList_New(vec.size());
        // memory should not leak
        // but seeking for better approach.
        object* buffer = (object*) malloc(sizeof(object));
        for(int i = 0; i < vec.size(); i++){
            object* ibobj = new (buffer) object(vec[i]);
            PyList_SetItem(obj, i, ibobj->ptr());
        }
        free(buffer);
        return obj;
    }

    static const PyTypeObject* get_pytype() {
        return &PyList_Type;
    }
};


template <typename KeyT, typename ValT, typename Map = std::map<KeyT, ValT>>
struct dict_map_converter {
    static void register_from_python_converter(){
        converter::registry::push_back(
            &dict_map_converter::convertible,
            &dict_map_converter::construct,
            boost::python::type_id<Map>());
    }

    static void register_to_python_converter(){
        boost::python::to_python_converter<Map, dict_map_converter<KeyT, ValT, Map>, true>();
    }

    static void register_bidirectional_converter(){
        register_from_python_converter();
        register_to_python_converter();
    }

    static void* convertible(PyObject *obj) {
        if(!PyDict_Check(obj))
            return nullptr;
        
        PyObject *key, *val;
        Py_ssize_t pos = 0;
        while (PyDict_Next(obj, &pos, &key, &val)) {
            extract<KeyT> key_e(key);
            extract<ValT> val_e(val);
            if(!key_e.check() || !val_e.check()){
                return nullptr;
            }
        }
        return obj;
    }

    static void construct(PyObject *obj,
                          boost::python::converter::rvalue_from_python_stage1_data *data)
    {
        void *storage = ((converter::rvalue_from_python_storage<Map> *)data)->storage.bytes;
        auto pmap = new (storage) Map();
        
        PyObject *key, *val;
        Py_ssize_t pos = 0;
        while (PyDict_Next(obj, &pos, &key, &val)) {
            (*pmap)[extract<KeyT>(key)] = extract<ValT>(val);
        }
        data->convertible = storage;
    }

    static PyObject* convert(const Map& m) {
        PyObject* obj = PyDict_New();
        // memory should not leak
        // but seeking for better approach.
        object* buffer = (object*) malloc(2 * sizeof(object));
        object* buffer_k = buffer + 0;
        object* buffer_v = buffer + 1;

        for(auto p : m) {
            // for C++ 11 support
            object* kobj = new (buffer_k) object(p.first);
            object* vobj = new (buffer_v) object(p.second);
            PyDict_SetItem(obj, kobj->ptr(), vobj->ptr());
        }
        free(buffer);
        return obj;
    }

    static const PyTypeObject* get_pytype() {
        return &PyDict_Type;
    }
};



template <typename... Ts>
struct tuple_tuple_converter {
    static void register_from_python_converter(){
        converter::registry::push_back(
            &tuple_tuple_converter::convertible,
            &tuple_tuple_converter::construct,
            boost::python::type_id<std::tuple<Ts...>>());
    }

    static void register_to_python_converter(){
        boost::python::to_python_converter<std::tuple<Ts...>, tuple_tuple_converter<Ts...>, true>();
    }

    static void register_bidirectional_converter(){
        register_from_python_converter();
        register_to_python_converter();
    }

    // index can also be placed in type parameters
    template<size_t index>
    static bool tuple_convertible(PyObject* obj){
        return true;
    }

    template<size_t index, typename P, typename ...Ps>
    static bool tuple_convertible(PyObject* obj){
        PyObject* iobj = PyTuple_GetItem(obj, index);
        extract<P> e(iobj);

        return e.check() && tuple_convertible<index + 1, Ps...>(obj);
    }

    static void* convertible(PyObject *obj) {
        if(!PyTuple_Check(obj))
            return nullptr;
        if(!tuple_convertible<0, Ts...>(obj))
            return nullptr;
        return obj;
    }

    template<size_t index>
    static void cpp_tuple_construct(PyObject* obj, std::tuple<Ts...>& t){}

    template<size_t index, typename P, typename ...Ps>
    static void cpp_tuple_construct(PyObject* obj, std::tuple<Ts...>& t){
        std::get<index>(t) = extract<P>(PyTuple_GetItem(obj, index));
        cpp_tuple_construct<index + 1, Ps...>(obj, t);
    }

    static void construct(PyObject* obj,
                          boost::python::converter::rvalue_from_python_stage1_data *data)
    {
        void *storage = ((converter::rvalue_from_python_storage<std::tuple<Ts...>>*)data)->storage.bytes;
        auto ptuple = new (storage) std::tuple<Ts...>();
        cpp_tuple_construct<0, Ts...>(obj, *ptuple);
        data->convertible = storage;
    }

    template<size_t index>
    static void py_tuple_construct(PyObject* obj, object* buffer, const std::tuple<Ts...>& tuple){}
    
    template<size_t index, typename P, typename ...Ps>
    static void py_tuple_construct(PyObject* obj, object* buffer, const std::tuple<Ts...>& tuple){
        object* iobj = new (buffer) object(std::get<index>(tuple));
        PyTuple_SetItem(obj, index, iobj->ptr());
        py_tuple_construct<index + 1, Ps...>(obj, buffer, tuple);
    }

    static PyObject* convert(const std::tuple<Ts...>& tuple) {
        PyObject* obj = PyTuple_New(std::tuple_size<std::tuple<Ts...>>::value);
        // memory should not leak
        // but seeking for better approach.
        object* buffer = (object*) malloc(sizeof(object));
        py_tuple_construct<0, Ts...>(obj, buffer, tuple);
        free(buffer);
        return obj;
    }

    static const PyTypeObject* get_pytype() {
        return &PyTuple_Type;
    }

};

#endif
