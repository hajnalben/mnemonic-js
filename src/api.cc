#include <nan.h>
#include <stdint.h>
#include <string.h>
#include "mnemonic.c"

NAN_METHOD(hash) {
  Nan::TypedArrayContents<char> p_input(info[0]);

  // This pointer points to a value before the array!!! So we must shift it!
  char* input = *p_input;

  size_t input_len = p_input.length();

  unsigned char shifted_input[input_len - 1];

  for (int i = 0; i < input_len; i++) {
    shifted_input[i] = input[i];
  }

  char format[] = "x x x x x|";

  if (info[1]->IsString()) {
    Nan::Utf8String param1(info[1]);
    std::string from = std::string(*param1);
    const char *cstr = from.c_str();

    strcpy(format, cstr);
  }

  char outbuf[40];

  mn_encode(shifted_input, sizeof(shifted_input), outbuf, sizeof(outbuf), format);

  // The first occurence of the terminating char
  const size_t block_len = strcspn(outbuf, "|") + 1;

  char output[block_len];
  memcpy(output, outbuf, block_len);

  // Add string terminating character
  output[block_len - 1] = '\0';

  auto result = Nan::New<v8::String>(output).ToLocalChecked();
  info.GetReturnValue().Set(result);
}

NAN_MODULE_INIT(Initialize) {
  NAN_EXPORT(target, hash);
}

NODE_MODULE(addon, Initialize)