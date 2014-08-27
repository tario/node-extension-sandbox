#include <node.h>

#include "main.hpp"

using namespace v8;


Persistent<FunctionTemplate> MyObject::constructor;

void MyObject::Init(Handle<Object> target) {
    HandleScope scope;

    Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
    Local<String> name = String::NewSymbol("MyObject");

    constructor = Persistent<FunctionTemplate>::New(tpl);
    // ObjectWrap uses the first internal field to store the wrapped pointer.
    constructor->InstanceTemplate()->SetInternalFieldCount(1);
    constructor->SetClassName(name);

    // Add all prototype methods, getters and setters here.
    NODE_SET_PROTOTYPE_METHOD(constructor, "value", Value);

    // This has to be last, otherwise the properties won't show up on the
    // object in JavaScript.
    target->Set(name, constructor->GetFunction());
}

MyObject::MyObject(int val)
    : ObjectWrap(),
      value_(val) {}


Handle<Value> MyObject::New(const Arguments& args) {
    HandleScope scope;

    if (!args.IsConstructCall()) {
        return ThrowException(Exception::TypeError(
            String::New("Use the new operator to create instances of this object."))
        );
    }

    if (args.Length() < 1) {
        return ThrowException(Exception::TypeError(
            String::New("First argument must be a number")));
    }

    // Creates a new instance object of this type and wraps it.
    MyObject* obj = new MyObject(args[0]->ToInteger()->Value());
    obj->Wrap(args.This());

    return args.This();
}


Handle<Value> MyObject::Value(const Arguments& args) {
    HandleScope scope;

    // Retrieves the pointer to the wrapped object instance.
    MyObject* obj = ObjectWrap::Unwrap<MyObject>(args.This());

    return scope.Close(Integer::New(obj->value_));
}

Persistent<FunctionTemplate> X::constructor;

X::X(){

}

Handle<Value> X::Foo(const Arguments& args) {
    if (args.Length() < 2) {
        return ThrowException(Exception::TypeError(
            String::New("First and second argument must be numbers")));
    }

    int a = args[0]->ToInteger()->Value();
    int b = args[1]->ToInteger()->Value();

    return Integer::New(a+b);
}

struct XBarAsyncData {
    Persistent<Function> callback;
};

void AsyncWork(uv_work_t* req) {

};

void AsyncAfter(uv_work_t* req) {
    HandleScope scope;
    XBarAsyncData* asyncData = static_cast<XBarAsyncData*>(req->data);

    TryCatch try_catch;
    asyncData->callback->Call(Context::GetCurrent()->Global(), 0, 0);
    if (try_catch.HasCaught()) {
        node::FatalException(try_catch);
    }

    asyncData->callback.Dispose();

    delete asyncData;
    delete req;
};


Handle<Value> X::Bar(const Arguments& args) {
    if (args.Length() < 1) {
        return ThrowException(Exception::TypeError(
            String::New("First argument should be a callback")));
    }

    Local<Function> callback = Local<Function>::Cast(args[0]);

    XBarAsyncData* asyncData = new XBarAsyncData;
    asyncData->callback = Persistent<Function>::New(callback);

    // This creates the work request struct.
    uv_work_t *req = new uv_work_t();
    req->data = asyncData;

    int status = uv_queue_work(uv_default_loop(), req, AsyncWork,
                               (uv_after_work_cb)AsyncAfter);


    return Undefined();
}

Handle<Value> X::New(const Arguments& args) {
    HandleScope scope;

    if (!args.IsConstructCall()) {
        return ThrowException(Exception::TypeError(
            String::New("Use the new operator to create instances of this object."))
        );
    }

    X* x = new X();
    x->Wrap(args.This());

    return args.This();
}


void X::Init(Handle<Object> target) {
    HandleScope scope;

    Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
    Local<String> name = String::NewSymbol("X");

    constructor = Persistent<FunctionTemplate>::New(tpl);
    // ObjectWrap uses the first internal field to store the wrapped pointer.
    constructor->InstanceTemplate()->SetInternalFieldCount(1);
    constructor->SetClassName(name);

    NODE_SET_PROTOTYPE_METHOD(constructor, "foo", Foo);
    NODE_SET_PROTOTYPE_METHOD(constructor, "bar", Bar);

    // This has to be last, otherwise the properties won't show up on the
    // object in JavaScript.
    target->Set(name, constructor->GetFunction());
}


void RegisterModule(Handle<Object> target) {
    MyObject::Init(target);
    X::Init(target);
}

NODE_MODULE(modulename, RegisterModule);
