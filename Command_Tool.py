def calculate_xor_checksum(data):
    checksum = 0
    for char in data:
        checksum ^= ord(char)
    return str(checksum).zfill(3)

def send_command():
    command = input("Enter the command to send: ")
    checksum = calculate_xor_checksum(command)
    padded_command = '*' + command + str(checksum) + '*'
    print("Command sent:", padded_command)
    print("XOR Checksum:", checksum)

def receive_command():
    received_command = input("Enter the received command: ")
    checksum = calculate_xor_checksum(received_command)
    padded_command = '#' + received_command + str(checksum) + '#'
    print("Received command:", padded_command)
    print("XOR Checksum:", checksum)

def main():
    while True:
        choice = input("Select an option - S for Send, R for Receive (Q to Quit): ").upper()

        if choice == 'S':
            send_command()
        elif choice == 'R':
            receive_command()
        elif choice == 'Q':
            print("Exiting program.")
            break
        else:
            print("Invalid choice. Please select either S or R.")

if __name__ == "__main__":
    main()
