
Build all firmware components and related items into one unified flash hexfile.

Dependencies:  
    python 2.7 (Other versions may work, but developed and tested with this version)  
    JLink installed and accessed via $PATH  
    intelhex package installed (hexmerge)

The hexmerge.py utility is part of the intelhex python package.  
It is assumed you already have pip installed.  
The command below will install intelhex (and hexmerge.py).  

     $ sudo pip install intelhex

**NOTE:** on linux you may need to change the permission on the hexmerge.py file.
```
  $ sudo chmod +x /usr/local/bin/hexmerge.py
  $ sudo chmod +x /usr/local/bin/hexmerge.pyc
```

**NOTE:** hexmerge can take quite a while to run; up to a minute or longer.

The sequence of operations are --  

      $ make clean
      $ make [debug|release]
      $ make flash
      
or  

     $ make clean  
     $ make flash  

Once the unified hexfile is build, you can repeately flash units with
        make flash

The products of this makefile are  

     unified.hex  
     unified_<timestamp>.hex  

The unitied_<timestamp>.hex is used for distribution.  
The timestamp portion is standard Unix time (GMT) and can be converted via  
      http://www.epochconverter.com
