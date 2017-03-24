{
  "targets": [
    {
      "target_name": "mnemonic",
      "sources": [ "src/api.cc" ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}