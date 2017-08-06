FTDI Latency update
=================================

The ftdi.latency.* application demonstrates two different features:

1. It shows how to make a program run as Admin.
   This involves creating a manifest file, named <filename>.manifest,
   and creating a resource file, <filename>.rc to tell build tools
   to include the manifest in the build.

2. It shows how to programmatically change the latency on the widely-used
   FTDI USB-Serial chip.  Without this change, the FTDI chip waits 16-msec
   after receiving a character, before passing it along to the receiver.
   This latency makes it very difficult to attain high data rates.
   At one of my recent jobs (2015 timeframe), we were running the UART
   at 3,000,000 baud !!  However, with the default FTDI latency, it was only
   possible to actually transfer data at about 14KB/second.
   By dropping the latency to 4-msec, we were able to sustain transfer
   rates of over 60KB/second.

Usage: ftdi.latency [options]
Options:
-l = list all FTDI devices and current latencies
-fN = set latency for all FTDI devices to N msec (1-30)

