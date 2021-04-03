# TinyDNS

This is a tiny DNS server with simple JSON config written in C.

## Features

* filesize is just only 20Kb!
* cache DNS queries for boost internet connection
* resolve own domains from config
* resolve multidomains like *.example.com
* server can work on IPv6 address

## Compile and Install

* if you use [Archlinux](https://archlinux.org), you may install [tinydns from AUR](https://aur.archlinux.org/packages/tinydns/)
* for compile just run `make`
* after install you need to write your IP address in `/etc/tinydns.conf`
* you may also use `systemctl` for start and stop service


## Changes in the fork

    IPV6 is optional
    Compiles with minilib to 8.6kB linked statically
    Ports are configurable


misc 2021


