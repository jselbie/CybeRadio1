#!/bin/csh

if (-e /local/home/wrek/www/sound_tmp/wrekout.au) then
        echo "Content-type: text/html"
        echo ""
        echo "<title> Sorry </title>"
        echo "<h3>There appears to be someone else using the web-demo"
        echo "at this time.  Please try again in a few minutes"
        exit
endif

/local/home/wrek/cgi-bin/wrek/cr1 -o >/local/home/wrek/www/sound_tmp/wrekout.au
/local/home/wrek/cgi-bin/wrek/raw2audio /local/home/wrek/www/sound_tmp/wrekout.a
u

echo "Content-type: audio/basic"
echo ""
cat /local/home/wrek/www/sound_tmp/wrekout.au
rm /local/home/wrek/www/sound_tmp/wrekout.au

