module SegReg_EX_LS#(
    parameter PC_RESET_VAL = 32'h0
)(
    input  logic [ 0:0] clk,
    input  logic [ 0:0] rstn,
    input  logic [ 0:0] stall,
    input  logic [ 0:0] flush,

    input  logic [31:0] pc_ex,
    input  logic [31:0] inst_ex,
    input  logic [31:0] alu_result_ex,
    input  logic [ 4:0] mem_access_ex,
    input  logic [ 0:0] wb_rf_sel_ex,
    input  logic [ 0:0] rf_we_ex,
    input  logic [ 0:0] commit_ex,

    //修改
    input  logic [ 0:0] csr_we_ex,
    input  logic [31:0] csr_wdata_ex,
    input  logic [ 0:0] ecall_ex,
    input  logic [ 0:0] mret_ex,

    output logic [31:0] pc_ls,
    output logic [31:0] inst_ls,
    output logic [31:0] alu_result_ls,
    output logic [ 4:0] mem_access_ls,
    output logic [ 0:0] wb_rf_sel_ls,
    output logic [ 0:0] rf_we_ls,
    output logic [ 0:0] commit_ls,

    //修改
    output  logic [ 0:0] csr_we_ls,
    output  logic [31:0] csr_wdata_ls,
    output  logic [ 0:0] ecall_ls,
    output  logic [ 0:0] mret_ls
);
    always_ff @(posedge clk) begin
        if(!rstn || flush) begin
            pc_ls           <= PC_RESET_VAL;
            inst_ls         <= 32'h13;
            alu_result_ls   <= 32'h0;
            mem_access_ls   <=  5'h0;
            wb_rf_sel_ls    <=  1'h0;
            rf_we_ls        <=  1'h0;
            commit_ls       <=  1'h0;
            //修改
            csr_we_ls       <=  1'h0;
            csr_wdata_ls    <=  32'h0;
            ecall_ls        <=  1'h0;
            mret_ls         <=  1'h0;
        end 
        else if(!stall) begin
            pc_ls           <= pc_ex;
            inst_ls         <= inst_ex;
            alu_result_ls   <= alu_result_ex;
            mem_access_ls   <= mem_access_ex;
            wb_rf_sel_ls    <= wb_rf_sel_ex;
            rf_we_ls        <= rf_we_ex;
            commit_ls       <= commit_ex;
            //修改
            csr_we_ls       <=csr_we_ex;
            csr_wdata_ls    <=csr_wdata_ex;
            ecall_ls        <=ecall_ex;
            mret_ls         <=mret_ex;
        end
    end
endmodule