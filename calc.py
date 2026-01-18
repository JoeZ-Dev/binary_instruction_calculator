
from collections import *



#-------------------------------------------
# USCC Headquarter's Instruction Set Architecture
#  System Design:
#   - Four function calculator
#   - Can only operate on numbers stored in registers
#   - Processor receives binary data as 32-bit strings
#   - Returns results to the terminal
#   - Can operate on 10-bit numbers (0 thru 1023)
#   - Results can be negative (5 - 10 = -5)
#  Instruction format:
#   - 32 bit's in length
#   - Binary data will come to the CPU as a string
#   - Registers (32 total on CPU, 0-indexed)
#      - 0 thru 21:  Available for number storage
#        - 0: Constant 0
#      - 22 thru 31: Available for history storage
# +=======+=======+=======+=======+=======+=======+=======+=======+
# | 0: 0  | 1:    | 2:    | 3:    | 4:    | 5:    | 6:    | 7:    |
# +-------+-------+-------+-------+-------+-------+-------+-------+
# | 8:    | 9:    |10:    |11:    |12:    |13:    |14:    |15:    |
# +-------+-------+-------+-------+-------+-------+-------+-------+
# |16:    |17:    |18:    |19:    |20:    |21:    |22: H0 |23: H1 |
# +-------+-------+-------+-------+-------+-------+-------+-------+
# |24: H2 |25: H3 |26: H4 |27: H5 |28: H6 |29: H7 |30: H8 |31: H9 |
# +=======+=======+=======+=======+=======+=======+=======+=======+
#   - Bits 0-5 are OPCODEs
#     - use variable 'opcode' in program
#   - Bits 6-10 & 11-15 are source register locations
#     - use variables 'source_one' and 'source_two' in program
#   - Bits 16-25 are reserved for adding a new value to the registers
#     - use variable 'store' in program
#   - Bits 26-31 are functions
#     - use variable 'function_code' in program
# +--------+----------+-------------------------------------+
# | OPCODE | FUNCTION | Definition                          |
# | 000000 |  100000  | Add two numbers from registers      |
# | 000000 |  100010  | Subtract two numbers from registers |
# | 000000 |  011000  | Multiply two numbers from registers |
# | 000000 |  011010  | Divide two numbers from registers   |
# | 000001 |  000000  | Store value to next register        |
# | 100001 |  000000  | Return previous calculation         |
# +--------+----------+-------------------------------------+

# Your code below here:

class UltraSuperCalculator:
  def __init__(self, name):
    self.name = name
    self.number_registers = [0]*22
    self.history_registers = [0] * 10
    self.numbers_index = 1
    self.history_index = 0
    self.temp_history_index = 0
    self.user_display = ""
    self.update_display(f"Welcome, {name}")
 
  def update_display(self, to_update):
    self.user_display = to_update
    print(self.user_display)

  def store_value_to_register(self, value_to_store):
    if self.numbers_index > 21:
      self.numbers_index = 1
    self.number_registers[0] = 0
    self.number_registers[self.numbers_index] = value_to_store
    print(f"Register {self.numbers_index}: {int(self.number_registers[self.numbers_index],2)}")
    self.numbers_index += 1
    
  def load_value_from_register(self, register_address):
    self.index = int(register_address,2)
    int_value = int(self.number_registers[self.index], 2)
    return int_value

  def store_to_history_register(self, result_to_store):
    if self.history_index > 9:
      self.history_index = 0
    self.history_registers[self.history_index] = bin(result_to_store)
    self.history_index += 1
    self.temp_history_index = self.history_index
  
  def add(self, address_num1, address_num2):
    num1 = self.load_value_from_register(address_num1)
    num2 = self.load_value_from_register(address_num2)
    calculated_value = num1 + num2
    return calculated_value

  def subtract(self, address_num1, address_num2):
    num1 = self.load_value_from_register(address_num1)
    num2 = self.load_value_from_register(address_num2)
    calculated_value = num1 - num2
    return calculated_value  

  def multiply(self, address_num1, address_num2):
    num1 = self.load_value_from_register(address_num1)
    num2 = self.load_value_from_register(address_num2)
    calculated_value = num1 * num2
    return calculated_value 

  def divide(self, address_num1, address_num2):
    num1 = self.load_value_from_register(address_num1)
    num2 = self.load_value_from_register(address_num2)
    if num1 == 0 or num2 == 0:
      return 0
    calculated_value = num1 // num2
    return calculated_value


  def get_last_calculation(self):
    self.temp_history_index -= 1
    last_value = "Last Result=" + str(int(self.history_registers[self.temp_history_index], 2))
    self.update_display(last_value)

  def binary_reader(self, instruction):
    if len(instruction) != 32:
      self.update_display("Invalid Instruction Length")
      return
    opcode = instruction[0:6]
    source_one = instruction[6:11]
    source_two = instruction[11:16]
    store = instruction[16:26]
    function_code = instruction[26:32]
    #print(f"Function Code: {function_code}")
    #print(f"Source 1: {int(source_one,2)} \nSource 2: {int(source_two,2)}")
    if opcode == '000001':
      self.store_value_to_register(store)
      return
    elif opcode == '100001':
      self.get_last_calculation()
      return
    elif opcode != '000000':
      self.update_display('Invalid OPCODE')
      return

    result = 0
    if function_code == '100000':
      action = 'Add'
      result = self.add(source_one, source_two)
    elif function_code == '100010':
      action = 'Subtract'
      result = self.subtract(source_one, source_two)
    elif function_code == '011000':
      action = 'Multiply'
      result = self.multiply(source_one, source_two)
    elif function_code == '011010':
      action = 'Divide'
      result = self.divide(source_one, source_two)
    else:
      self.update_display('Invalid Function Code')
    
    
    self.store_to_history_register(result)
    self.update_display(f"{action} Result: {result}")

    



your_calc_name = UltraSuperCalculator("James")
#calc1.store_value_to_register("101")
#calc1.binary_reader("1234567812345678123456781234567")
#calc1.binary_reader("12345678123456781234567812345678")
#calc1.binary_reader("00000078123456781234567812100000")


# Adds 5 and 10 to number registers
your_calc_name.binary_reader("00000100000000000000000101000000")
your_calc_name.binary_reader("00000100000000000000001010000000")

# Adds/Subtracts/Multiplies/Divides 5 and 10 from registers
your_calc_name.binary_reader("00000000001000100000000000100000")
your_calc_name.binary_reader("00000000001000100000000000100010")
your_calc_name.binary_reader("00000000001000100000000000011000")
your_calc_name.binary_reader("00000000001000100000000000011010")

# Gets the last three calculations
your_calc_name.binary_reader("10000100000000000000000000000000")
your_calc_name.binary_reader("10000100000000000000000000000000")
your_calc_name.binary_reader("10000100000000000000000000000000")
