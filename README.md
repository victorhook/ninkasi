# Ninkasi

The beer delivery drone



## Setup

python3 -m pymavlink.tools.mavgen --lang=C --wire-protocol=2.0 --output=generated/include/mavlink/include/mavlink/v2.0 message_definitions/v1.0/ardupilotmega.xml 

Compiling
```
g++-aarch64-linux-gnu
```

```
sudo apt-get install libopencv-dev libzbar-dev python3-pip

```