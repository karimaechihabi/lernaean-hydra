// myobject.h
#ifndef jsADS_H
#define jsADS_H

#include <node.h>
#include <node_object_wrap.h>

extern "C" {
    #include <ads.h>
}

class ADS {
private:
	isax_index *index;
public:
	ADS(char* database_directory);
  ~ADS();
	float query(ts_type* ts, ts_type* result);
	float approximate_query(ts_type* ts, ts_type* result);
  unsigned long long get_total_records();
  unsigned long long get_loaded_records();
  void print_info();
  meminfo memory_utilization_info();
};

class jsADS : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> exports);

 private:
  explicit jsADS(char* value);

  ~jsADS();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void PrintInfo(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void GetTotalRecords(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void GetLoadedRecords(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Query(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void ApproximateQuery(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Close(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void GetMemoryUtilizationInfo(const v8::FunctionCallbackInfo<v8::Value>& args);

  static v8::Persistent<v8::Function> constructor;
  char* database_directory_;
  ADS *index;
};

#endif
