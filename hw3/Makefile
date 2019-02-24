TARGETS=player ringmaster
call: $(TARGETS)
clean:
	rm -f $(TARGETS)

client: player.cpp
	g++ -g -o $@ $<

server: ringmaster.cpp
	g++ -g -o $@ $<

