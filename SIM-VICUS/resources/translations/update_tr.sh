#!/bin/bash

export PATH=~/Qt/5.11.3/gcc_64/bin:$PATH
lupdate ../../projects/Qt/SIM-VICUS.pro

linguist SIM-VICUS_de.ts

