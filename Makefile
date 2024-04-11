CC = gcc
CC_STANDARD = c11
CLIENT_ONE = client1
CLIENT_TWO = client2
OBJECT_NAME_1 = $(CLIENT_ONE).o
OBJECT_NAME_2 = $(CLIENT_TWO).o
RUN_CODE = $(CC) $(LDFLAGS) -std=$(CC_STANDARD)

all: $(CLIENT_TWO) $(CLIENT_ONE)
	
run: $(CLIENT)
	./$(CLIENT)

clean:
	@rm -f $(CLIENT_ONE) $(CLIENT_TWO) *.o && echo "Cleaning successfully" || echo "It's not successfully"

$(CLIENT_ONE):$(OBJECT_NAME_1)
	@$(RUN_CODE) -o $@ $^ 

$(CLIENT_TWO):$(OBJECT_NAME_2)
	@$(RUN_CODE) -o $@ $^ 

$(OBJECT_NAME_1): $(CLIENT_ONE).c
	@$(RUN_CODE) -c $< -o $@

$(OBJECT_NAME_2): $(CLIENT_TWO).c
	@$(RUN_CODE) -c $< -o $@

.PHONY:	all run clean $(CLIENT_ONE) $(CLIENT_TWO)