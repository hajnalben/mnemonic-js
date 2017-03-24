#include <nan.h>
#include <stdint.h>
#include "mnemonic.c"

NAN_METHOD(hash) {
  Nan::TypedArrayContents<uint8_t> data(info[0]);

  char input[data.length()];

  for (size_t i=0; i<data.length(); i++) {
    input[i] = (*data)[i];
  }

  char format[] = "x x x x x|";
  char outbuf[40];

  mn_encode(input, data.length(), outbuf, sizeof(outbuf), format);

  auto message = Nan::New<v8::String>(outbuf).ToLocalChecked();
  info.GetReturnValue().Set(message);
}

NAN_MODULE_INIT(Initialize) {
  NAN_EXPORT(target, hash);
}

NODE_MODULE(addon, Initialize)