`timescale 1ns/1ps
`include "./include/config.sv"
module Exp_Commit(
    input  logic [ 9:0] priv_vec,
    output logic [31:0] exp_code
);
    always_comb begin
        exp_code = 32'h0;
        if(priv_vec[`ECALL]) begin
            exp_code = 32'hb;
        end
        else if(priv_vec[`BAD_PC]) begin
            exp_code = 32'hc;
        end
        else if(priv_vec[`BAD_INST]) begin
            exp_code = 32'h2;
        end
        else if(priv_vec[`PC_FAULT]) begin
            exp_code = 32'h1;
        end
        else if(priv_vec[`EBREAK]) begin
            exp_code = 32'h3;
        end
    end
endmodule
