#include "jsads.h"

extern "C" {
    #include <string.h>
    #include <ads.h>
}
using namespace v8;

Persistent<Function> jsADS::constructor;

jsADS::jsADS(char* database_directory) : database_directory_(database_directory) {
  this->index = new ADS(database_directory);
}

jsADS::~jsADS() {
  delete this->index;
}

void jsADS::Init(Handle<Object> exports) {
  Isolate* isolate = Isolate::GetCurrent();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "jsADS"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  NODE_SET_PROTOTYPE_METHOD(tpl, "printInfo", PrintInfo);
  NODE_SET_PROTOTYPE_METHOD(tpl, "query", Query);
  NODE_SET_PROTOTYPE_METHOD(tpl, "approximateQuery", ApproximateQuery);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getTotalRecords", GetTotalRecords);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getLoadedRecords", GetLoadedRecords);
  NODE_SET_PROTOTYPE_METHOD(tpl, "close", Close);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getMemoryUtilizationInfo", GetMemoryUtilizationInfo);
  
  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "jsADS"),
               tpl->GetFunction());
}

void jsADS::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  if (args.IsConstructCall()) {
    v8::Local<v8::String> string = args[0]->ToString();
    const int length = string->Utf8Length() + 1;  // Add one for trailing zero byte.
    char* value = new char[length];
    string->WriteOneByte((unsigned char *) value, /* start */ 0, length);
    jsADS* obj = new jsADS(value);
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    const int argc = 1;
    Local<Value> argv[argc] = { args[0] };
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(cons->NewInstance(argc, argv));
  }
}

void jsADS::PrintInfo(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  jsADS* obj = ObjectWrap::Unwrap<jsADS>(args.Holder());
  obj->index->print_info();
  
  // obj->database_directory_ = "test2";
  // const uint8_t* octets = reinterpret_cast<const uint8_t*>(obj->database_directory_);
  // v8::Local<v8::String> latin1 = v8::String::NewFromOneByte(isolate, octets);
  // args.GetReturnValue().Set(latin1);
}

void jsADS::Close(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  jsADS* obj = ObjectWrap::Unwrap<jsADS>(args.Holder());
  delete obj->index;
}

void jsADS::Query(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
 
  HandleScope scope(isolate);
  jsADS* obj = ObjectWrap::Unwrap<jsADS>(args.Holder());

  // Run query
  Local<Object>  array  = args[0]->ToObject();
  int length = array->Get(String::NewFromUtf8(isolate, "length"))->ToObject()->Uint32Value();
  ts_type *ts = new ts_type[length];
  ts_type *answer = new ts_type[length];
  for(int i = 0; i < length; i++)
  {
      v8::Local<v8::Value> element = array->Get(i);
      ts[i] = element->NumberValue();
  }
  
  float distance = obj->index->query(ts, answer);
  
  Handle<Array> output = Array::New(isolate, length + 1);
  output->Set(0, Number::New(isolate, distance));
  for(int i = 1; i <= length; i++)
  {
        output->Set(i, Number::New(isolate, answer[i-1]));
  }

  delete ts;
  delete answer;

  args.GetReturnValue().Set(output);
}

void jsADS::ApproximateQuery(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
 
  HandleScope scope(isolate);
  jsADS* obj = ObjectWrap::Unwrap<jsADS>(args.Holder());

  // Run query
  Local<Object>  array  = args[0]->ToObject();
  int length = array->Get(String::NewFromUtf8(isolate, "length"))->ToObject()->Uint32Value();
  ts_type *ts = new ts_type[length];
  ts_type *answer = new ts_type[length];
  for(int i = 0; i < length; i++)
  {
      v8::Local<v8::Value> element = array->Get(i);
      ts[i] = element->NumberValue();
  }
  
  float distance = obj->index->approximate_query(ts, answer);
  
  Handle<Array> output = Array::New(isolate, length + 1);
  output->Set(0, Number::New(isolate, distance));
  for(int i = 1; i <= length; i++)
  {
        output->Set(i, Number::New(isolate, answer[i-1]));
  }

  delete ts;
  delete answer;

  args.GetReturnValue().Set(output);
}

void jsADS::GetTotalRecords(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  jsADS* obj = ObjectWrap::Unwrap<jsADS>(args.Holder());

  unsigned long long total_records = obj->index->get_total_records();
  
  args.GetReturnValue().Set(Number::New(isolate, total_records)); 
}

void jsADS::GetLoadedRecords(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  jsADS* obj = ObjectWrap::Unwrap<jsADS>(args.Holder());

  unsigned long long loaded_records = obj->index->get_loaded_records();

  args.GetReturnValue().Set(Number::New(isolate, loaded_records)); 
}

void jsADS::GetMemoryUtilizationInfo(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
 
  HandleScope scope(isolate);
  jsADS* obj = ObjectWrap::Unwrap<jsADS>(args.Holder());

  meminfo memory_utilization_info = obj->index->memory_utilization_info();
  
  Handle<Array> output = Array::New(isolate, 5);
  output->Set(0, Number::New(isolate, memory_utilization_info.mem_tree_structure));
  output->Set(1, Number::New(isolate, memory_utilization_info.mem_summaries));
  output->Set(2, Number::New(isolate, memory_utilization_info.mem_data));
  output->Set(3, Number::New(isolate, memory_utilization_info.disk_data_full));
  output->Set(4, Number::New(isolate, memory_utilization_info.disk_data_partial));
  
  args.GetReturnValue().Set(output);
}


ADS::ADS(char *database_directory) {
    this->index = index_read(database_directory);
    cache_sax_file(this->index);
}

ADS::~ADS() {
    fprintf(stderr, "Clearing buffers and saving index.\n");
    flush_all_leaf_buffers(this->index, TMP_ONLY_CLEAN);
    index_write(this->index);
    isax_index_destroy(this->index, NULL);
}

float ADS::query(ts_type *ts, ts_type *answer) {
    ts_type * paa = new ts_type[this->index->settings->paa_segments];
    paa_from_ts(ts, paa, this->index->settings->paa_segments, 
                         this->index->settings->ts_values_per_paa_segment);
    query_result result = exact_search_serial(ts, paa, this->index, 100000000, 1);
    delete paa;

    memcpy(answer, this->index->answer, this->index->settings->timeseries_size * sizeof(ts_type));
    
    return result.distance;
}

float ADS::approximate_query(ts_type *ts, ts_type *answer) {
    ts_type * paa = new ts_type[this->index->settings->paa_segments];
    paa_from_ts(ts, paa, this->index->settings->paa_segments, 
                         this->index->settings->ts_values_per_paa_segment);
    query_result result = approximate_search(ts, paa, this->index);
    if(result.node == NULL) {
      result = refine_answer(ts, paa, index, result, 100000000, 1);
    }

    delete paa;

    memcpy(answer, this->index->answer, this->index->settings->timeseries_size * sizeof(ts_type));
    
    return result.distance;
}

void ADS::print_info() {
    print_settings(this->index->settings);
}

unsigned long long ADS::get_total_records() {
    return this->index->total_records;
}

unsigned long long ADS::get_loaded_records() {
    return this->index->loaded_records;
}

meminfo ADS::memory_utilization_info() {
  return get_memory_utilization_info(this->index);
}

void InitAll(Handle<Object> exports) {
  jsADS::Init(exports);
}

NODE_MODULE(addon, InitAll)
