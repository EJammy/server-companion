## Clangd config
```
local default_capabilities = require('cmp_nvim_lsp').default_capabilities
local on_attach = require('lsp').on_attach

require'lspconfig'.clangd.setup {
   cmd = {'clangd', '--query-driver', '/usr/bin/arm-none-eabi-gcc'},
   capabilities = default_capabilities(),
   on_attach = on_attach
}
```
# Features
## Secure* remote wake
1. Client

1. Client requests challenge number from Pico server
2. Client computes and sends hash(n+key)
3. Pico server confirms key and broadcast WOL
## Connection to main server
Transmit logs

# Protocol choice
## UDP
- Fast
- If no response, simply retry
## TCP
- Guarantees packet delivery
- Probably use this to send log
## HTTP
- Useless and annoying
- Browser/JS support
- Probably just get a Raspberry Pi zero for this

One shot request protocol
Challenge–response authentication
Modes:
Time stamp mode

sha-256 copied from https://github.com/amosnier/sha-2/tree/master

# Dumps
https://www.raspberrypi.com/documentation/microcontrollers/pico-series.html
https://datasheets.raspberrypi.com/pico/raspberry-pi-pico-c-sdk.pdf

Other ideas:
- libssh, Paramiko
