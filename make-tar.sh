#!/bin/bash
make -C bugs/Debug clean
make -C bugs/Release clean
cd ..
rm -f bugs-master.tar
tar --exclude=.metadata -cpf bugs-master.tar bugs
cd bugs
ls -lh ../bugs-master.tar
