#!/bin/sh -e

apt-get dist-upgrade --allow-unauthenticated -qq
apt-get clean -qq
apt-get autoremove -qq
