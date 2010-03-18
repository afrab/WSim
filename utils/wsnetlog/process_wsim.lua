#!/opt/local/bin/lua

-- Author: Michael Hauspie <Michael.Hauspie@lifl.fr>
-- Created: 10/03/2010
-- Time-stamp: <2010-03-12 16:25:01 (hauspie)>
-- Version: $Id$

-- Defines the preamble to use for detecting a frame start
-- can be modified by calling lua with -e 'radio_preamble="XXXXXX"'
-- Default is to use the CC2420 preamble/SFD
if not radio_preamble
then
   radio_preamble = "00000000A7"
end


-- This function transforms a string to an array of byte (unsigned)
function string_to_bytes(s)
   bytes = {}
   for i=1,#s/2,1
   do
      st = string.sub(s, i*2-1, i*2)
      number = tonumber(st, 16)
      table.insert(bytes, number)
   end
   return bytes
end

-- This function extracts the bytes from the wsnet log file Given in
-- parameter. If no parameter is given, it process stdin 
-- Returns an array which includes, for each nodes, an array of structures
-- describing the bytes read from the wsim log.
-- bytes[ip][i] <- ith byte of the log for IP ip which is described using the following structure
-- byte.byte <- value of the byte (unsigned)
-- byte.time <- timestamp of the byte as reported by wsim
function extract_bytes(...)
   local packets = {}
   
   for line in io.lines(unpack(arg))
   do
      if string.find(line,"TX")
      then
	 time, id, data = string.match(line, "(%d*),.*ip:(%d*).*data:(%x*)")
	 if not id or not time or not data
	 then
	    return nil
	 end
	 if not packets[id]
	 then
	    packets[id] = {}
	 end
	 value = {}
	 value.byte=tonumber(data,16)
	 value.time=tonumber(time)
	 table.insert(packets[id], value)
      end
   end
   return packets
end


-- Decode a stream of bytes, each byte beeing a struct of {byte,time}
-- Returns an array of packets, each packet beeing a structure with the following fields
-- data -> an array of bytes (unsigned)
-- time -> the estimated time of the packet (the time of the last byte is used)
function decode_packets(bytes)
   local decoded_packets = {}
   local state = "SEARCH_PREAMBLE" -- possible states are: SEARCH_PREAMBLE, SEARCH_SIZE, PACKET_BODY
   local current_packet = nil

   local actions = { 
      SEARCH_PREAMBLE =
	 function (byte)
	    preamb = preamb or ""
	    if (#preamb >= #radio_preamble) 
	    then 
	       -- strip the old first byte
	       preamb = string.sub(preamb, 3, -1)
	    end
	    preamb = preamb .. string.format("%02X", byte.byte)
	    if (preamb == radio_preamble) -- Detected start of packet
	    then
	       current_packet = {}
	       current_packet.data = string_to_bytes(radio_preamble)
	       state = "SEARCH_SIZE"
	    end
	 end,
      SEARCH_SIZE = 
	 function (byte)
	    table.insert(current_packet.data, byte.byte)
	    current_packet.size = byte.byte
	    state = "PACKET_BODY"
	 end,
      PACKET_BODY = 
	 function (byte)
	    table.insert(current_packet.data, byte.byte)
	    if ((#(current_packet.data) - (#radio_preamble/2) - 1 )== current_packet.size) -- all bytes have been read
	    then
	       current_packet.time = byte.time
	       table.insert(decoded_packets, current_packet)
	       state = "SEARCH_PREAMBLE"
	    end
	 end,
   }

   -- Execute the state machine with each bytes as input
   for i=1,(#bytes),1
   do
      actions[state](bytes[i])
   end
   return decoded_packets
end


-- Main program begins here

if arg[1] and arg[1] == "--help"
then
   io.stderr:write("Usage: " .. arg[0] .. " [input_file]\n")
   io.stderr:write([[
If no input file is given, standard input is used.
If launched with lua command, you can also set another value 
for the preamble.
Ex: lua -e 'radio_preamble="CAFEDECA"' ]] .. arg[0] .. " [input_file]\n")
   io.stderr:write("The packets are ordered using the transmit time of the LAST byte\n")
   return 1
end

sorted_packets = {}
extracted_bytes = extract_bytes(...) -- ... is the command line args

if not extracted_bytes
then
   io.stderr:write("Input file is not a wsnet1 log file\n")
   return 1
end

for id, bytes in pairs(extracted_bytes)
do
   packets = decode_packets(bytes)
   for k, packet in pairs(packets)
   do
      table.insert(sorted_packets, packet)
   end
end


table.sort(sorted_packets, function (p1, p2) return p1.time < p2.time end )

for k, packet in pairs(sorted_packets)
do
   for i=1,#(packet.data),1
   do
      if (i == #(packet.data))
      then
	 io.write(string.format("%02X\n", packet.data[i]))
      else
	 io.write(string.format("%02X-", packet.data[i]))
      end
   end
end
