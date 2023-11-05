`include "./include/config.sv"

// Lab4 TODO: implement the Exp_Commit module for ecall instruction
// you need to generate exception code for `syscall from machine mode`
module Exp_Commit(
    input   logic [ 0:0] ecall_wb, //是否是ecall指令
    output  logic [ 3:0] exp_code,
    output  logic [ 0:0] exp_valid
);
    always_comb begin
        exp_valid = 1'b0;
        exp_code = 4'b0;
        if (ecall_wb == 1'b1) begin
            exp_valid = 1'b1;
            exp_code = 4'b1011;
        end
    end
endmodule