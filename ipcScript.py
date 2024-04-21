from multiprocessing import shared_memory

try:
    try:
        # Attempt to open the existing shared memory segment
        shared_mem = shared_memory.SharedMemory(name='shared_memory', create=False)
    except Exception:
        print("shared memory does not exist")

    # Read data from the shared memory
    data = shared_mem.buf[:].tobytes()
    print("Received from C++ file:", data.decode())
    #Perform some operations
    shared_mem.buf[:] = b'\0' * len(shared_mem.buf)

    message = input("Enter message to be send to server: ")
    
    binary_data = message.encode('utf-8')
    length = len(message)
    shared_mem.buf[:length] = binary_data

    # Clean up the shared memory
    shared_mem.close()
    # shared_memory_obj.unlink()  # Remove the shared memory segment
except FileNotFoundError:
    print("Error: Shared memory segment not found.")
except PermissionError:
    print("Error: Permission denied when accessing shared memory.")
except Exception as e:
    print("Error accessing shared memory:", type(e).__name__, "-", e)
