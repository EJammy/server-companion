## Configuration setup
Simply make a copy of example_config.h and edit the file.
```
cp pico/example_config.h pico/config.h
```
## Generating keys
First, generate the key
```
head -c 64 /dev/random > secret_key.bin
```

Next, get the key in hex and paste the output to `pico/config.h`:
```
hexdump -v -e '16/1 "0x%02x, " "\\\n"' secret_key.bin
```
