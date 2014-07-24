#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "registers.h"
#include "cpu.h"

struct ExecutionContext {
    uint8_t cycles;
    z80cpu_t *cpu;
    union {
        uint8_t opcode;
        struct {
            uint8_t z : 3;
            uint8_t y : 3;
            uint8_t x : 2;
        };
        struct {
            uint8_t   : 3;
            uint8_t q : 1;
            uint8_t p : 2;
        };
    };
    uint8_t (*n)(struct ExecutionContext *);
    uint16_t (*nn)(struct ExecutionContext *);
    int8_t (*d)(struct ExecutionContext *);
};

z80cpu_t* cpu_init(void) {
    z80cpu_t *cpu = calloc(1, sizeof(z80cpu_t));
    z80iodevice_t nullDevice = { NULL, NULL, NULL };
    int i;
    for (i = 0; i < 0x100; i++) {
        cpu->devices[i] = nullDevice;
    }
    return cpu;
}

void cpu_raise_interrupt(z80cpu_t  *cpu) {
    cpu->INT_pending = 1;
}

void cpu_free(z80cpu_t *cpu) {
    free(cpu);
}

uint16_t cpu_read_register_word(z80cpu_t *cpu, registers reg_to_read) {
	uint16_t return_value = 0;
	switch(reg_to_read) {
	case  AF:
		return_value = cpu->registers.AF;
		break;
	case _AF:
		return_value = cpu->registers._AF;
		break;
	case  BC:
		return_value = cpu->registers.BC;
		break;
	case _BC:
		return_value = cpu->registers._BC;
		break;
	case  DE:
		return_value = cpu->registers.DE;
		break;
	case _DE:
		return_value = cpu->registers._DE;
		break;
	case  HL:
		return_value = cpu->registers.HL;
		break;
	case _HL:
		return_value = cpu->registers._HL;
		break;
	default:
		return_value = -1;
	}

	return_value = hook_on_register_read(cpu->hook, reg_to_read, return_value);

	return return_value;
}

uint8_t cpu_read_register_byte(z80cpu_t *cpu, registers reg_to_read) {
	uint8_t return_value = 0;

	switch(reg_to_read) {
	case A:
		return_value = cpu->registers.A;
		break;
	case F:
		return_value = cpu->registers.F;
		break;
	case B:
		return_value = cpu->registers.B;
		break;
	case C:
		return_value = cpu->registers.C;
		break;
	case D:
		return_value = cpu->registers.D;
		break;
	case E:
		return_value = cpu->registers.E;
		break;
	case H:
		return_value = cpu->registers.H;
		break;
	case L:
		return_value = cpu->registers.L;
		break;
	case I:
		return_value = cpu->registers.I;
		break;
	case R:
		return_value = cpu->registers.R;
		break;
	case IXH:
		return_value = cpu->registers.IXH;
		break;
	case IXL:
		return_value = cpu->registers.IXL;
		break;
	case IYH:
		return_value = cpu->registers.IYH;
		break;
	case IYL:
		return_value = cpu->registers.IYL;
		break;
	default:
		return_value = -1;
	}

	return_value = (uint8_t) hook_on_register_read(cpu->hook, reg_to_read, return_value);

	return return_value;
}
uint16_t cpu_write_register_word(z80cpu_t *cpu, registers reg_to_read, uint16_t value) {
	uint16_t return_value = value;
	return_value = hook_on_register_write(cpu->hook, reg_to_read, value);

	switch(reg_to_read) {
	case  AF:
		cpu->registers.AF = return_value;
		break;
	case _AF:
		cpu->registers._AF = return_value;
		break;
	case  BC:
		cpu->registers.BC = return_value;
		break;
	case _BC:
		cpu->registers._BC = return_value;
		break;
	case  DE:
		cpu->registers.DE = return_value;
		break;
	case _DE:
		cpu->registers._DE = return_value;
		break;
	case  HL:
		cpu->registers.HL = return_value;
		break;
	case _HL:
		cpu->registers._HL = return_value;
		break;
	default:
		break;
	}

	return return_value;
}

uint8_t cpu_write_register_byte(z80cpu_t *cpu, registers reg_to_read, uint8_t value) {
	uint8_t return_value = value;
	return_value = (uint8_t)hook_on_register_write(cpu->hook, reg_to_read, value);

	switch(reg_to_read) {
	case A:
		cpu->registers.A = return_value;
		break;
	case F:
		cpu->registers.F = return_value;
		break;
	case B:
		cpu->registers.B = return_value;
		break;
	case C:
		cpu->registers.C = return_value;
		break;
	case D:
		cpu->registers.D = return_value;
		break;
	case E:
		cpu->registers.E = return_value;
		break;
	case H:
		cpu->registers.H = return_value;
		break;
	case L:
		cpu->registers.L = return_value;
		break;
	case I:
		cpu->registers.I = return_value;
		break;
	case R:
		cpu->registers.R = return_value;
		break;
	case IXH:
		cpu->registers.IXH = return_value;
		break;
	case IXL:
		cpu->registers.IXL = return_value;
		break;
	case IYH:
		cpu->registers.IYH = return_value;
		break;
	case IYL:
		cpu->registers.IYL = return_value;
		break;
	default:
		break;
	}

	return return_value;
}

uint8_t cpu_read_byte(z80cpu_t *cpu, uint16_t address) {
    return cpu->read_byte(cpu->memory, address);
}

void cpu_write_byte(z80cpu_t *cpu, uint16_t address, uint8_t value) {
    cpu->write_byte(cpu->memory, address, value);
}

uint16_t cpu_read_word(z80cpu_t *cpu, uint16_t address) {
    return cpu->read_byte(cpu->memory, address) | (cpu->read_byte(cpu->memory, address + 1) << 8);
}

void cpu_write_word(z80cpu_t *cpu, uint16_t address, uint16_t value) {
    cpu->write_byte(cpu->memory, address, value & 0xFF);
    cpu->write_byte(cpu->memory, address + 1, value >> 8);
}

void push(z80cpu_t *cpu, uint16_t value) {
    cpu_write_word(cpu, cpu->registers.SP - 2, value);
    cpu->registers.SP -= 2;
}

uint16_t pop(z80cpu_t *cpu) {
    uint16_t a = cpu_read_word(cpu, cpu->registers.SP);
    cpu->registers.SP += 2;
    return a;
}

uint8_t read_n(struct ExecutionContext *context) {
    return cpu_read_byte(context->cpu, context->cpu->registers.PC++);
}

uint16_t read_nn(struct ExecutionContext *context) {
    uint16_t a = cpu_read_word(context->cpu, context->cpu->registers.PC);
    context->cpu->registers.PC += 2;
    return a;
}

int8_t read_d(struct ExecutionContext *context) {
    return (int8_t)cpu_read_byte(context->cpu, context->cpu->registers.PC++);
}

uint8_t HorIHr(struct ExecutionContext *context) {
    if (context->cpu->prefix == 0xDD) {
        return context->cpu->registers.IXH;
    } else if (context->cpu->prefix == 0xFD) {
        return context->cpu->registers.IYH;
    } else {
        return context->cpu->registers.H;
    }
}

uint8_t HorIHw(struct ExecutionContext *context, uint8_t value) {
    if (context->cpu->prefix == 0xDD) {
        context->cpu->registers.IXH = value;
    } else if (context->cpu->prefix == 0xFD) {
        context->cpu->registers.IYH = value;
    } else {
        context->cpu->registers.H = value;
    }
    return value;
}

uint8_t LorILr(struct ExecutionContext *context) {
    if (context->cpu->prefix == 0xDD) {
        return context->cpu->registers.IXL;
    } else if (context->cpu->prefix == 0xFD) {
        return context->cpu->registers.IYL;
    } else {
        return context->cpu->registers.L;
    }
}

uint8_t LorILw(struct ExecutionContext *context, uint8_t value) {
    if (context->cpu->prefix == 0xDD) {
        context->cpu->registers.IXL = value;
    } else if (context->cpu->prefix == 0xFD) {
        context->cpu->registers.IYL = value;
    } else {
        context->cpu->registers.L = value;
    }
    return value;
}

uint16_t HLorIr(struct ExecutionContext *context) {
    if (context->cpu->prefix == 0xDD) {
        return context->cpu->registers.IX;
    } else if (context->cpu->prefix == 0xFD) {
        return context->cpu->registers.IY;
    } else {
        return context->cpu->registers.HL;
    }
}

uint16_t HLorIw(struct ExecutionContext *context, uint16_t value) {
    if (context->cpu->prefix == 0xDD) {
        context->cpu->registers.IX = value;
    } else if (context->cpu->prefix == 0xFD) {
        context->cpu->registers.IY = value;
    } else {
        context->cpu->registers.HL = value;
    }
    return value;
}

uint8_t indHLorIr(struct ExecutionContext *context) {
    // This function erases the prefix early so that the next read (H or L) does not
    // use IXH or IXL
    if (context->cpu->prefix == 0xDD) {
        context->cycles += 9;
        context->cpu->prefix = 0;
        return cpu_read_byte(context->cpu, context->cpu->registers.IX + read_d(context));
    } else if (context->cpu->prefix == 0xFD) {
        context->cycles += 9;
        context->cpu->prefix = 0;
        return cpu_read_byte(context->cpu, context->cpu->registers.IY + read_d(context));
    } else {
        return cpu_read_byte(context->cpu, context->cpu->registers.HL);
    }
}

uint8_t indHLorIw(struct ExecutionContext *context, uint8_t value) {
    if (context->cpu->prefix == 0xDD) {
        context->cycles += 9;
        context->cpu->prefix = 0;
        cpu_write_byte(context->cpu, context->cpu->registers.IX + read_d(context), value);
    } else if (context->cpu->prefix == 0xFD) {
        context->cycles += 9;
        context->cpu->prefix = 0;
        cpu_write_byte(context->cpu, context->cpu->registers.IY + read_d(context), value);
    } else {
        cpu_write_byte(context->cpu, context->cpu->registers.HL, value);
    }
    return value;
}

uint8_t read_r(int i, struct ExecutionContext *context) {
    int8_t d;
    switch (i) {
    case 0: return context->cpu->registers.B;
    case 1: return context->cpu->registers.C;
    case 2: return context->cpu->registers.D;
    case 3: return context->cpu->registers.E;
    case 4: return HorIHr(context);
    case 5: return LorILr(context);
    case 6:
        context->cycles += 3;
        if (context->cpu->prefix == 0xDD) {
            context->cycles += 8;
            d = context->d(context);
            return cpu_read_byte(context->cpu, context->cpu->registers.IX + d);
        } else if (context->cpu->prefix == 0xFD) {
            context->cycles += 8;
            d = context->d(context);
            return cpu_read_byte(context->cpu, context->cpu->registers.IY + d);
        } else {
            return cpu_read_byte(context->cpu, context->cpu->registers.HL);
        }
    case 7: return context->cpu->registers.A;
    }
    return 0; // This should never happen
}

uint8_t write_r(int i, uint8_t value, struct ExecutionContext *context) {
    int8_t d;
    switch (i) {
    case 0: return context->cpu->registers.B = value;
    case 1: return context->cpu->registers.C = value;
    case 2: return context->cpu->registers.D = value;
    case 3: return context->cpu->registers.E = value;
    case 4: return HorIHw(context, value);
    case 5: return LorILw(context, value);
    case 6:
        context->cycles += 3;
        if (context->cpu->prefix == 0xDD) {
            context->cycles += 4;
            d = context->d(context);
            cpu_write_byte(context->cpu, context->cpu->registers.IY + d, value);
        } else if (context->cpu->prefix == 0xFD) {
            context->cycles += 4;
            d = context->d(context);
            cpu_write_byte(context->cpu, context->cpu->registers.IX + d, value);
        } else {
            cpu_write_byte(context->cpu, context->cpu->registers.HL, value);
        }
        return value;
    case 7: return context->cpu->registers.A = value;
    }
    return 0; // This should never happen
}

uint16_t read_rp(int i, struct ExecutionContext *context) {
    switch (i) {
    case 0: return context->cpu->registers.BC;
    case 1: return context->cpu->registers.DE;
    case 2: return HLorIr(context);
    case 3: return context->cpu->registers.SP;
    }
    return 0; // This should never happen
}

uint16_t write_rp(int i, uint16_t value, struct ExecutionContext *context) {
    switch (i) {
    case 0: return context->cpu->registers.BC = value;
    case 1: return context->cpu->registers.DE = value;
    case 2: return HLorIw(context, value);
    case 3: return context->cpu->registers.SP = value;
    }
    return 0; // This should never happen
}

uint16_t read_rp2(int i, struct ExecutionContext *context) {
    switch (i) {
    case 0: return context->cpu->registers.BC;
    case 1: return context->cpu->registers.DE;
    case 2: return HLorIr(context);
    case 3: return context->cpu->registers.AF;
    }
    return 0; // This should never happen
}

uint16_t write_rp2(int i, uint16_t value, struct ExecutionContext *context) {
    switch (i) {
    case 0: return context->cpu->registers.BC = value;
    case 1: return context->cpu->registers.DE = value;
    case 2: return HLorIw(context, value);
    case 3: return context->cpu->registers.AF = value;
    }
    return 0; // This should never happen
}

uint8_t read_cc(int i, struct ExecutionContext *context) {
    z80registers_t *r = &context->cpu->registers;
    switch (i) {
    case 0: return !r->flags.Z;
    case 1: return  r->flags.Z;
    case 2: return !r->flags.C;
    case 3: return  r->flags.C;
    case 4: return !r->flags.PV;
    case 5: return  r->flags.PV;
    case 6: return !r->flags.N;
    case 7: return  r->flags.N;
    }
    return 0; // This should never happen
}

void daa(struct ExecutionContext *context) {
    z80registers_t *r = &context->cpu->registers;
    uint8_t msn = r->A >> 4;
    uint8_t lsn = r->A & 0xF;
    uint8_t c = r->flags.C;
    if (lsn > 9 || r->flags.H) {
        r->A += 0x06;
        r->flags.C = 1;
    }
    if (msn > 9 || c) {
        r->A += 0x60;
        r->flags.C = 1;
    }

    r->flags.Z = r->A == 0;
    r->flags.S = (r->A & 0x80) == 0x80;
    updateParity(r, r->A);
}

void execute_alu(int i, uint8_t v, struct ExecutionContext *context) {
    uint8_t old;
    context->cycles += 4;
    z80registers_t *r = &context->cpu->registers;
    switch (i) {
    case 0: // ADD A, v
        old = r->A;
        r->A += v;
        updateFlags(r, old, r->A, 0);
        break;
    case 1: // ADC A, v
        old = r->A;
        r->A += v + r->flags.C;
        updateFlags(r, old, r->A, 0);
        break;
    case 2: // SUB v
        old = r->A;
        r->A -= v;
        updateFlags_subtraction(r, old, r->A, 0);
        break;
    case 3: // SBC v
        old = r->A;
        r->A -= v + r->flags.C;
        updateFlags_subtraction(r, old, r->A, 0);
        break;
    case 4: // AND v
        old = r->A;
        r->A &= v;
        updateFlags_parity(r, old, r->A, 0);
        r->flags.C = r->flags.N = 0;
        r->flags.H = 1;
        break;
    case 5: // XOR v
        old = r->A;
        r->A ^= v;
        updateFlags_parity(r, old, r->A, 0);
        r->flags.C = r->flags.N = r->flags.H = 0;
        break;
    case 6: // OR v
        old = r->A;
        r->A |= v;
        updateFlags_parity(r, old, r->A, 0);
        r->flags.C = r->flags.N = r->flags.H = 0;
        break;
    case 7: // CP v
        old = r->A - v;
        updateFlags_subtraction(r, r->A, old, 0);
        break;
    }
}

void execute_rot(int y, int z, struct ExecutionContext *context) {
    uint8_t r = read_r(z, context);
    uint8_t old_r = r;
    uint8_t old_7 = (r & 0x80) > 0;
    uint8_t old_0 = (r & 1) > 0;
    uint8_t old_c = context->cpu->registers.flags.C > 0;
    z80cpu_t *cpu = context->cpu;
    switch (y) { 
    case 0: // RLC r[z]
        r <<= 1; r |= old_7;
        write_r(z, r, context);
        updateFlags_parity(&context->cpu->registers, old_r, r, 0);
        cpu->registers.flags.C = old_7;
        cpu->registers.flags.N = cpu->registers.flags.H = 0;
        break;
    case 1: // RRC r[z]
        r >>= 1; r |= old_0 << 7;
        write_r(z, r, context);
        updateFlags_parity(&context->cpu->registers, old_r, r, 0);
        cpu->registers.flags.C = old_0;
        cpu->registers.flags.N = cpu->registers.flags.H = 0;
        break;
    case 2: // RL r[z]
        r <<= 1; r |= old_c;
        write_r(z, r, context);
        updateFlags_parity(&context->cpu->registers, old_r, r, 0);
        cpu->registers.flags.C = old_7;
        cpu->registers.flags.N = cpu->registers.flags.H = 0;
        break;
    case 3: // RR r[z]
        r >>= 1; r |= old_c << 7;
        write_r(z, r, context);
        updateFlags_parity(&context->cpu->registers, old_r, r, 0);
        cpu->registers.flags.C = old_0;
        cpu->registers.flags.N = cpu->registers.flags.H = 0;
        break;
    case 4: // SLA r[z]
        r <<= 1;
        write_r(z, r, context);
        updateFlags_parity(&context->cpu->registers, old_r, r, 0);
        cpu->registers.flags.C = old_7;
        cpu->registers.flags.N = cpu->registers.flags.H = 0;
        break;
    case 5: // SRA r[z]
        r >>= 1;
        r |= old_7 << 7;
        write_r(z, r, context);
        updateFlags_parity(&context->cpu->registers, old_r, r, 0);
        cpu->registers.flags.C = old_0;
        cpu->registers.flags.N = cpu->registers.flags.H = 0;
        break;
    case 6: // SLL r[z]
        r <<= 1; r |= 1;
        write_r(z, r, context);
        updateFlags_parity(&context->cpu->registers, old_r, r, 0);
        cpu->registers.flags.C = old_7;
        cpu->registers.flags.N = cpu->registers.flags.H = 0;
        break;
    case 7: // SRL r[z]
        r >>= 1;
        write_r(z, r, context);
        updateFlags_parity(&context->cpu->registers, old_r, r, 0);
        cpu->registers.flags.C = old_0;
        cpu->registers.flags.N = cpu->registers.flags.H = 0;
        break;
    }
}

void execute_bli(int y, int z, struct ExecutionContext *context) {
    z80registers_t *r = &context->cpu->registers;
    z80iodevice_t ioDevice;
    uint8_t new;
    switch (y) {
    case 4:
        switch (z) {
        case 0: // LDI
            context->cycles += 12;
            cpu_write_byte(context->cpu, r->DE++, cpu_read_byte(context->cpu, r->HL++));
            r->flags.PV = !--r->BC;
            r->flags.N = r->flags.H = 0;
            break;
        case 1: // CPI
            context->cycles += 12;
            new = cpu_read_byte(context->cpu, r->HL++);
            updateFlags_except(r, r->A, r->A - new, 0, FLAG_C);
            r->flags.PV = !--r->BC;
            r->flags.N = 1;
            break;
        case 2: // INI
            context->cycles += 12;
            ioDevice = context->cpu->devices[r->C];
            if (ioDevice.read_in != NULL) {
                cpu_write_byte(context->cpu, r->HL, ioDevice.read_in(ioDevice.device));
            }
            r->HL++;
            r->flags.Z = !--r->B;
            r->flags.N = 1;
            break;
        case 3: // OUTI
            context->cycles += 12;
            ioDevice = context->cpu->devices[r->C];
            if (ioDevice.write_out != NULL) {
                ioDevice.write_out(ioDevice.device, cpu_read_byte(context->cpu, r->HL));
            }
            r->HL++;
            r->flags.Z = !--r->B;
            r->flags.N = 1;
            break;
        }
        break;
    case 5:
        switch (z) {
        case 0: // LDD
            context->cycles += 12;
            cpu_write_byte(context->cpu, r->DE--, cpu_read_byte(context->cpu, r->HL--));
            r->flags.PV = !--r->BC;
            r->flags.N = r->flags.H = 0;
            break;
        case 1: // CPD
            context->cycles += 12;
            new = cpu_read_byte(context->cpu, r->HL--);
            updateFlags_except(r, r->A, r->A - new, 0, FLAG_C);
            r->flags.PV = !--r->BC;
            r->flags.N = 1;
            break;
        case 2: // IND
            context->cycles += 12;
            ioDevice = context->cpu->devices[r->C];
            if (ioDevice.read_in != NULL) {
                cpu_write_byte(context->cpu, r->HL, ioDevice.read_in(ioDevice.device));
            }
            r->HL--;
            r->flags.Z = !--r->B;
            r->flags.N = 1;
            break;
        case 3: // OUTD
            context->cycles += 12;
            ioDevice = context->cpu->devices[r->C];
            if (ioDevice.write_out != NULL) {
                ioDevice.write_out(ioDevice.device, cpu_read_byte(context->cpu, r->HL));
            }
            r->HL--;
            r->flags.Z = !--r->B;
            r->flags.N = 1;
            break;
        }
        break;
    case 6:
        switch (z) {
        case 0: // LDIR
            context->cycles += 12;
            cpu_write_byte(context->cpu, r->DE++, cpu_read_byte(context->cpu, r->HL++));
            r->BC--;
            r->flags.N = r->flags.H = r->flags.PV = 0;
            if (r->BC) {
                context->cycles += 5;
                r->PC -= 2;
            }
            break;
        case 1: // CPIR
            context->cycles += 12;
            new = cpu_read_byte(context->cpu, r->HL++);
            updateFlags_except(r, r->A, r->A - new, 0, FLAG_C);
            r->flags.PV = !--r->BC;
            r->flags.N = 1;
            if (r->BC && !r->flags.Z) {
                context->cycles += 5;
                r->PC -= 2;
            }
            break;
        case 2: // INIR
            context->cycles += 12;
            ioDevice = context->cpu->devices[r->C];
            if (ioDevice.read_in != NULL) {
                cpu_write_byte(context->cpu, r->HL, ioDevice.read_in(ioDevice.device));
            }
            r->HL++;
            r->flags.Z = !--r->B;
            r->flags.N = 1;
            if (!r->flags.Z) {
                context->cycles += 5;
                r->PC -= 2;
            }
            break;
        case 3: // OTIR
            context->cycles += 12;
            ioDevice = context->cpu->devices[r->C];
            if (ioDevice.write_out != NULL) {
                ioDevice.write_out(ioDevice.device, cpu_read_byte(context->cpu, r->HL));
            }
            r->HL++;
            r->flags.Z = !--r->B;
            r->flags.N = 1;
            if (!r->flags.Z) {
                context->cycles += 5;
                r->PC -= 2;
            }
            break;
        }
        break;
    case 7:
        switch (z) {
        case 0: // LDDR
            context->cycles += 12;
            cpu_write_byte(context->cpu, r->DE--, cpu_read_byte(context->cpu, r->HL--));
            r->BC--;
            r->flags.N = r->flags.H = r->flags.PV = 0;
            if (r->BC) {
                context->cycles += 5;
                r->PC -= 2;
            }
            break;
        case 1: // CPDR
            context->cycles += 12;
            new = cpu_read_byte(context->cpu, r->HL--);
            updateFlags_except(r, r->A, r->A - new, 0, FLAG_C);
            r->flags.PV = !r->BC--;
            r->flags.N = 1;
            if (r->BC && !r->flags.Z) {
                context->cycles += 5;
                r->PC -= 2;
            }
            break;
        case 2: // INDR
            context->cycles += 12;
            ioDevice = context->cpu->devices[r->C];
            if (ioDevice.read_in != NULL) {
                cpu_write_byte(context->cpu, r->HL, ioDevice.read_in(ioDevice.device));
            }
            r->HL--;
            r->flags.Z = !--r->B;
            r->flags.N = 1;
            if (!r->flags.Z) {
                context->cycles += 5;
                r->PC -= 2;
            }
            break;
        case 3: // OTDR
            context->cycles += 12;
            ioDevice = context->cpu->devices[r->C];
            if (ioDevice.write_out != NULL) {
                ioDevice.write_out(ioDevice.device, cpu_read_byte(context->cpu, r->HL));
            }
            r->HL--;
            r->flags.Z = !--r->B;
            r->flags.N = 1;
            if (!r->flags.Z) {
                context->cycles += 5;
                r->PC -= 2;
            }
            break;
        }
        break;
    }
}

void execute_im(int y, struct ExecutionContext *context) {
    switch (y) {
        case 0: context->cpu->int_mode = 0; break;
        case 1: context->cpu->int_mode = 0; break; // 0/1
        case 2: context->cpu->int_mode = 1; break;
        case 3: context->cpu->int_mode = 2; break;
        case 4: context->cpu->int_mode = 0; break;
        case 5: context->cpu->int_mode = 0; break; // 0/1
        case 6: context->cpu->int_mode = 1; break;
        case 7: context->cpu->int_mode = 2; break;
    }
}

void handle_interrupt(struct ExecutionContext *context) {
    // Note: Should we consider doing a proper raise/acknowledge mechanism
    // with interrupting devices? It's probably not entirely required but it
    // might be nice to follow the actual behavior more closely.
    z80cpu_t *cpu = context->cpu;
    z80registers_t *r = &cpu->registers;
    switch (cpu->int_mode) {
    case 0:
        fprintf(stderr, "Warning: Interrupt mode 0 is not currently supported.\n");
        break;
    case 1:
        context->cycles += 13;
        push(cpu, r->PC);
        r->PC = 0x38;
        cpu->IFF1 = 0;
        cpu->IFF2 = 0;
        break;
    case 2:
        context->cycles += 19;
        push(cpu, r->PC);
        r->PC = r->I * 256 + cpu->bus;
        cpu->IFF1 = 0;
        cpu->IFF2 = 0;
        break;
    }
}

int cpu_execute(z80cpu_t *cpu, int cycles) {
    struct ExecutionContext context;
    context.cpu = cpu;
    while (cycles > 0 || cpu->prefix != 0) {
        context.cycles = 0;
        if (cpu->IFF2) {
            if (cpu->IFF_wait) {
                cpu->IFF_wait = 0;
            } else {
                if (cpu->INT_pending) {
                    cpu->INT_pending = 0;
                    cpu->halted = 0;
                    handle_interrupt(&context);
                    goto exit_loop;
                }
            }
        }
        if (cpu->halted) {
            context.cycles += 4;
            goto exit_loop;
        }

        context.opcode = cpu_read_byte(cpu, cpu->registers.PC++);
        context.n = read_n;
        context.nn = read_nn;
        context.d = read_d;
        int8_t d; uint16_t nn;
        uint8_t old; uint16_t old16;
        uint8_t new; uint16_t new16;
        uint8_t prefix = 0;
        z80registers_t *r = &cpu->registers;
        z80iodevice_t ioDevice;

        uint8_t old_r = r->R;
        r->R++;
        r->R &= 0x7F;
        r->R |= old_r & 0x80;

        if (cpu->prefix == 0xCB || cpu->prefix == 0xED) {
            switch (cpu->prefix) {
            case 0xCB:
                switch (context.x) {
                case 0: // rot[y] r[z]
                    context.cycles += 4;
                    execute_rot(context.y, context.z, &context);
                    break;
                case 1: // BIT y, r[z]
                    context.cycles += 4;
                    old = read_r(context.z, &context);
                    r->flags.H = 1;
                    r->flags.N = 0;
                    cpu->registers.flags.Z = (old & (1 << context.y)) > 0;
                    break;
                case 2: // RES y, r[z]
                    context.cycles += 4;
                    old = read_r(context.z, &context);
                    old &= ~(1 << context.y);
                    write_r(context.z, old, &context);
                    break;
                case 3: // SET y, r[z]
                    context.cycles += 4;
                    old = read_r(context.z, &context);
                    old |= 1 << context.y;
                    write_r(context.z, old, &context);
                    break;
                }
                cpu->prefix = 0;
                break;
            case 0xED:
                switch (context.x) {
                case 1:
                    switch (context.z) {
                    case 0:
                        if (context.y == 6) { // IN (C)
                            context.cycles += 8;
                            ioDevice = cpu->devices[cpu->registers.C];
                            if (ioDevice.read_in != NULL) {
                                new = ioDevice.read_in(ioDevice.device);
                                updateFlags_except(r, new, new, 0, FLAG_C);
                                r->flags.H = r->flags.N = 0;
                            }
                        } else { // IN r[y], (C)
                            context.cycles += 8;
                            ioDevice = cpu->devices[r->C];
                            if (ioDevice.read_in != NULL) {
                                new = ioDevice.read_in(ioDevice.device);
                                old = read_r(context.y, &context);
                                write_r(context.y, new, &context);
                                updateFlags_withOptions(r, old, new, 0, 1, 0, FLAG_C);
                                r->flags.N = r->flags.H = 0;
                            }
                        }
                        break;
                    case 1:
                        if (context.y == 6) { // OUT (C), 0
                            // This instruction outputs 0 for NMOS z80s, and 0xFF for CMOS z80s.
                            // TIs are the CMOS variant. Most emulators do *not* emulate this
                            // correctly, but I have verified through my own research that the
                            // correct value to output is 0xFF.
                            context.cycles += 8;
                            ioDevice = cpu->devices[r->C];
                            if (ioDevice.write_out != NULL) {
                                ioDevice.write_out(ioDevice.device, 0xFF);
                            }
                        } else { // OUT (C), r[y]
                            context.cycles += 8;
                            ioDevice = cpu->devices[r->C];
                            if (ioDevice.write_out != NULL) {
                                ioDevice.write_out(ioDevice.device, read_r(context.y, &context));
                            }
                        }
                        break;
                    case 2:
                        if (context.q == 0) { // SBC HL, rp[p]
                            context.cycles += 11;
                            old16 = r->HL;
                            r->HL -= read_rp(context.p, &context) + r->flags.C;
                            updateFlags_subtraction(r, old16, r->HL, 1);
                        } else { // ADC HL, rp[p]
                            context.cycles += 11;
                            old16 = r->HL;
                            r->HL += read_rp(context.p, &context) + r->flags.C;
                            updateFlags(r, old16, r->HL, 1);
                        }
                        break;
                    case 3:
                        if (context.q == 0) { // LD (nn), rp[p]
                            context.cycles += 16;
                            cpu_write_word(cpu, context.nn(&context), read_rp(context.p, &context));
                        } else { // LD rp[p], (nn)
                            context.cycles += 16;
                            write_rp(context.p, cpu_read_word(cpu, context.nn(&context)), &context);
                        }
                        break;
                    case 4: // NEG
                        context.cycles += 4;
                        old = r->A;
                        r->A = -r->A;
                        updateFlags_subtraction(r, old, r->A, 0);
                        r->flags.C = old != 0;
                        r->flags.PV = old == 0x80;
                        break;
                    case 5:
                        if (context.y == 1) { // RETI
                            // Note: Intentionally not implemented, not relevant for TI devices
                        } else { // RETN
                            // Note: Intentionally not implemented, not relevant for TI devices
                        }
                        break;
                    case 6: // IM im[y]
                        context.cycles += 4;
                        execute_im(context.y, &context);
                        break;
                    case 7:
                        switch (context.y) {
                        case 0: // LD I, A
                            context.cycles += 5;
                            r->I = r->A;
                            break;
                        case 1: // LD R, A
                            context.cycles += 5;
                            r->R = r->A;
                            break;
                        case 2: // LD A, I
                            context.cycles += 5;
                            old = r->A;
                            r->A = r->I;
                            updateFlags_except(r, old, r->A, 0, FLAG_C);
                            r->flags.H = r->flags.N = 0;
                            r->flags.PV = cpu->IFF2;
                            break;
                        case 3: // LD A, R
                            context.cycles += 5;
                            old = r->A;
                            r->A = r->R;
                            updateFlags_except(r, old, r->A, 0, FLAG_C);
                            r->flags.H = r->flags.N = 0;
                            r->flags.PV = cpu->IFF2;
                            break;
                        case 4: // RRD
                            context.cycles += 14;
                            old = r->A;
                            old16 = cpu_read_word(cpu, r->HL);
                            r->A = old16 & 0xFF;
                            old16 >>= 8;
                            old16 |= old << 8;
                            cpu_write_word(cpu, r->HL, old16);
                            break;
                        case 5: // RLD
                            context.cycles += 14;
                            old = r->A;
                            old16 = cpu_read_word(cpu, r->HL);
                            r->A = old16 >> 8;
                            old16 <<= 8;
                            old16 |= old;
                            cpu_write_word(cpu, r->HL, old16);
                            break;
                        default: // NOP (invalid instruction)
                            context.cycles += 4;
                            break;
                        }
                        break;
                    }
                    break;
                case 2:
                    if (context.y >= 4) { // bli[y,z]
                        execute_bli(context.y, context.z, &context);
                    } else { // NONI (invalid instruction)
                        context.cycles += 4;
                        cpu->IFF_wait = 1;
                    }
                    break;
                default: // NONI (invalid instruction)
                    context.cycles += 4;
                    cpu->IFF_wait = 1;
                    break;
                }
                cpu->prefix = 0;
                break;
            }
        } else {
            switch (context.x) {
            case 0:
                switch (context.z) {
                case 0:
                    switch (context.y) {
                    case 0: // NOP
                        context.cycles += 4;
                        break;
                    case 1: // EX AF, AF'
                        context.cycles += 4;
                        exAFAF(r);
                        break;
                    case 2: // DJNZ d
                        context.cycles += 8;
                        d = context.d(&context);
                        r->B--;
                        if (r->B != 0) {
                            context.cycles += 5;
                            r->PC += d;
                        }
                        break;
                    case 3: // JR d
                        context.cycles += 12;
                        d = context.d(&context);
                        r->PC += d;
                        break;
                    case 4:
                    case 5:
                    case 6:
                    case 7: // JR cc[y-4], d
                        context.cycles += 7;
                        d = context.d(&context);
                        if (read_cc(context.y - 4, &context)) {
                            context.cycles += 5;
                            r->PC += d;
                        }
                        break;
                    }
                    break;
                case 1:
                    switch (context.q) {
                    case 0: // LD rp[p], nn
                        context.cycles += 10;
                        write_rp(context.p, context.nn(&context), &context);
                        break;
                    case 1: // ADD HL, rp[p]
                        context.cycles += 11;
                        old16 = HLorIr(&context);
                        new16 = HLorIw(&context, old16 + read_rp(context.p, &context));
                        updateFlags_except(r, old16, new16, 1, FLAG_Z | FLAG_S | FLAG_PV);
                        break;
                    }
                    break;
                case 2:
                    switch (context.q) {
                    case 0:
                        switch (context.p) {
                        case 0: // LD (BC), A
                            context.cycles += 7;
                            cpu_write_byte(cpu, r->BC, r->A);
                            break;
                        case 1: // LD (DE), A
                            context.cycles += 7;
                            cpu_write_byte(cpu, r->DE, r->A);
                            break;
                        case 2: // LD (nn), HL
                            context.cycles += 16;
                            cpu_write_word(cpu, context.nn(&context), HLorIr(&context));
                            break;
                        case 3: // LD (nn), A
                            context.cycles += 13;
                            cpu_write_byte(cpu, context.nn(&context), r->A);
                            break;
                        }
                        break;
                    case 1:
                        switch (context.p) {
                        case 0: // LD A, (BC)
                            context.cycles += 7;
                            r->A = cpu_read_byte(cpu, r->BC);
                            break;
                        case 1: // LD A, (DE)
                            context.cycles += 7;
                            r->A = cpu_read_byte(cpu, r->DE);
                            break;
                        case 2: // LD HL, (nn)
                            context.cycles += 16;
                            HLorIw(&context, cpu_read_word(cpu, context.nn(&context)));
                            break;
                        case 3: // LD A, (nn)
                            context.cycles += 13;
                            old16 = context.nn(&context);
                            r->A = cpu_read_byte(cpu, old16);
                            break;
                        }
                        break;
                    }
                    break;
                case 3:
                    switch (context.q) {
                    case 0: // INC rp[p]
                        context.cycles += 6;
                        write_rp(context.p, read_rp(context.p, &context) + 1, &context);
                        break;
                    case 1: // DEC rp[p]
                        context.cycles += 6;
                        write_rp(context.p, read_rp(context.p, &context) - 1, &context);
                        break;
                    }
                    break;
                case 4: // INC r[y]
                    context.cycles += 4;
                    old = read_r(context.y, &context);
                    new = write_r(context.y, old + 1, &context);
                    updateFlags_except(r, old, new, 0, FLAG_C);
                    r->flags.PV = old == 0x7F;
                    break;
                case 5: // DEC r[y]
                    context.cycles += 4;
                    old = read_r(context.y, &context);
                    new = write_r(context.y, old - 1, &context);
                    updateFlags_withOptions(r, old, new, 0, 1, 0, FLAG_C);
                    r->flags.PV = old == 0x80;
                    break;
                case 6: // LD r[y], n
                    context.cycles += 7;
                    write_r(context.y, context.n(&context), &context);
                    break;
                case 7:
                    switch (context.y) {
                    case 0: // RLCA
                        context.cycles += 4;
                        old = (r->A & 0x80) > 0;
                        r->flags.C = old;
                        r->A <<= 1;
                        r->A |= old;
                        r->flags.N = r->flags.H = 0;
                        break;
                    case 1: // RRCA
                        context.cycles += 4;
                        old = (r->A & 1) > 0;
                        r->flags.C = old;
                        r->A >>= 1;
                        r->A |= old << 7;
                        r->flags.N = r->flags.H = 0;
                        break;
                    case 2: // RLA
                        context.cycles += 4;
                        old = r->flags.C;
                        r->flags.C = (r->A & 0x80) > 0;
                        r->A <<= 1;
                        r->A |= old;
                        r->flags.N = r->flags.H = 0;
                        break;
                    case 3: // RRA
                        context.cycles += 4;
                        old = r->flags.C;
                        r->flags.C = (r->A & 1) > 0;
                        r->A >>= 1;
                        r->A |= old << 7;
                        r->flags.N = r->flags.H = 0;
                        break;
                    case 4: // DAA
                        context.cycles += 4;
                        old = r->A;
                        daa(&context);
                        break;
                    case 5: // CPL
                        context.cycles += 4;
                        r->A = ~r->A;
                        r->flags.N = r->flags.H = 1;
                        break;
                    case 6: // SCF
                        context.cycles += 4;
                        r->flags.C = 1;
                        r->flags.N = r->flags.H = 0;
                        break;
                    case 7: // CCF
                        context.cycles += 4;
                        r->flags.H = r->flags.C;
                        r->flags.C = !r->flags.C;
                        r->flags.N = 0;
                        break;
                    }
                    break;
                }
                break;
            case 1:
                if (context.z == 6 && context.y == 6) { // HALT
                    context.cycles += 4;
                    cpu->halted = 1;
                } else { // LD r[y], r[z]
                    context.cycles += 4;
                    write_r(context.y, read_r(context.z, &context), &context);
                }
                break;
            case 2: // ALU[y] r[z]
                execute_alu(context.y, read_r(context.z, &context), &context);
                break;
            case 3:
                switch (context.z) {
                case 0: // RET cc[y]
                    context.cycles += 5;
                    if (read_cc(context.y, &context)) {
                        r->PC = pop(cpu);
                        context.cycles += 6;
                    }
                    break;
                case 1:
                    switch (context.q) {
                    case 0: // POP rp2[p]
                        context.cycles += 10;
                        write_rp2(context.p, pop(cpu), &context);
                        break;
                    case 1:
                        switch (context.p) {
                        case 0: // RET
                            context.cycles += 10;
                            r->PC = pop(cpu);
                            break;
                        case 1: // EXX
                            context.cycles += 4;
                            exx(&cpu->registers);
                            break;
                        case 2: // JP HL
                            context.cycles += 4;
                            r->PC = HLorIr(&context);
                            break;
                        case 3: // LD SP, HL
                            context.cycles += 6;
                            r->SP = HLorIr(&context);
                            break;
                        }
                        break;
                    }
                    break;
                case 2: // JP cc[y], nn
                    context.cycles += 10;
                    nn = context.nn(&context);
                    if (read_cc(context.y, &context)) {
                        r->PC = nn;
                    }
                    break;
                case 3:
                    switch (context.y) {
                    case 0: // JP nn
                        context.cycles += 10;
                        r->PC = context.nn(&context);
                        break;
                    case 1: // 0xCB prefixed opcodes
                        context.cycles += 4;
                        prefix = 0xCB;
                        break;
                    case 2: // OUT (n), A
                        context.cycles += 11;
                        ioDevice = cpu->devices[context.n(&context)];
                        if (ioDevice.write_out != NULL) {
                            ioDevice.write_out(ioDevice.device, r->A);
                        }
                        break;
                    case 3: // IN A, (n)
                        context.cycles += 11;
                        ioDevice = cpu->devices[context.n(&context)];
                        if (ioDevice.read_in != NULL) {
                            r->A = ioDevice.read_in(ioDevice.device);
                        }
                        break;
                    case 4: // EX (SP), HL
                        context.cycles += 19;
                        old16 = cpu_read_word(cpu, r->SP);
                        cpu_write_word(cpu, r->SP, HLorIr(&context));
                        HLorIw(&context, old16);
                        break;
                    case 5: // EX DE, HL
                        context.cycles += 4;
                        exDEHL(&cpu->registers);
                        break;
                    case 6: // DI
                        context.cycles += 4;
                        cpu->IFF1 = 0;
                        cpu->IFF2 = 0;
                        break;
                    case 7: // EI
                        context.cycles += 4;
                        cpu->IFF1 = 1;
                        cpu->IFF2 = 1;
                        cpu->IFF_wait = 1;
                        break;
                    }
                    break;
                case 4: // CALL cc[y], nn
                    context.cycles += 10;
                    nn = context.nn(&context);
                    if (read_cc(context.y, &context)) {
                        context.cycles += 7;
                        push(cpu, r->PC);
                        r->PC = nn;
                    }
                    break;
                case 5:
                    switch (context.q) {
                    case 0: // PUSH r2p[p]
                        context.cycles += 11;
                        push(cpu, read_rp2(context.p, &context));
                        break;
                    case 1:
                        switch (context.p) {
                        case 0: // CALL nn
                            context.cycles += 17;
                            nn = context.nn(&context);
                            push(cpu, r->PC);
                            r->PC = nn;
                            break;
                        case 1: // 0xDD prefixed opcodes
                            context.cycles += 4;
                            prefix = 0xDD;
                            break;
                        case 2: // 0xED prefixed opcodes
                            context.cycles += 4;
                            prefix = 0xED;
                            break;
                        case 3: // 0xFD prefixed opcodes
                            context.cycles += 4;
                            prefix = 0xFD;
                            break;
                        }
                        break; 
                    }
                    break;
                case 6: // alu[y] n
                    execute_alu(context.y, context.n(&context), &context);
                    break;
                case 7: // RST y*8
                    context.cycles += 11;
                    push(context.cpu, r->PC + 2);
                    r->PC = context.y * 8;
                    break;
                }
                break;
            }
        }

        cpu->prefix = prefix;

exit_loop:
        cycles -= context.cycles;
        if (context.cycles == 0) {
            fprintf(stderr, "Warning: Unrecognized instruction %02X.\n", context.opcode);
            cycles--;
        }
    }
    return cycles;
}
