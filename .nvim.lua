local default_capabilities = require('cmp_nvim_lsp').default_capabilities
local on_attach = require('lsp').on_attach

require'lspconfig'.clangd.setup {
   cmd = {'clangd', '--query-driver', '/usr/bin/arm-none-eabi-gcc'},
   capabilities = default_capabilities(),
   on_attach = on_attach
}
