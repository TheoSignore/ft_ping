# ft_ping

42 school `ft_ping` project.

---

**Disclaimer: This project was developped for a purely pedagogic purpose. It is not meant to replace your system's ping command.**

---

## Summary

A partial rewriting of [inetutils2.0's ping command](https://www.gnu.org/software/inetutils/manual/html_node/ping-invocation.html#ping-invocation) written in c for linux.

*The objective of this project was to use and understand the [IP](https://www.rfc-editor.org/rfc/rfc791#section-3.1) and [ICMP](https://www.rfc-editor.org/rfc/rfc792.html) protcols.*

---

## Building and usage
### Dependencies
This project requires `Make` and the `gcc` compiler.
The executable requires capabalities to open [RAW sockets](https://www.man7.org/linux/man-pages/man7/raw.7.html)
### Building
```sh
git clone https://github.com/TheoSignore/ft_ping
cd ft_ping
make
sudo setcap cap_net_raw,cap_net_admin=eip ./ft_ping
```
### Usage
Display help: `./ft_ping -?`

`./ft_ping [-v] <HOST>`

`-v`: optional verbose mode

`HOST`: can be
  1. a valid IPv4 address
  2. a fully qualified domain name (FQDN) associated with a valid IPv4 address

---

## Description

Sends ICMP_ECHO requests to a network host and uses the possible replies to compute loss and latency statistics.

---

Written by [Théo Signore](https://github.com/TheoSignore)


