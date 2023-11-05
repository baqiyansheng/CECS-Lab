`include "./include/config.sv"
module CSR(
    input  logic [ 0:0] clk,
    input  logic [ 0:0] rstn,
    input  logic [11:0] raddr,
    input  logic [11:0] waddr,
    //修改
    input  logic [ 0:0] exp_valid,
    input  logic [ 3:0] exp_code,
    input  logic [ 0:0] mret_wb,

    input  logic [ 0:0] we,
    input  logic [31:0] wdata,
    input  logic [31:0] pc_wb,
    output logic [31:0] rdata,
    //修改
    output logic [31:0] mtvec_global,
    output logic [31:0] mepc_global
    // Lab4 TODO: you need to add some input or output pors to implement CSRs' special functions
);
    import "DPI-C" function void set_csr_ptr(input logic [31:0] m1 [], input logic [31:0] m2 [], input logic [31:0] m3 [], input logic [31:0] m4 []);
    reg [31:0] mstatus;
    always_ff @(posedge clk) begin
        if(!rstn) begin
            mstatus <= 32'h0;
        end
        // Lab4 TODO: implement mstatus
        else begin
            if(we && waddr == `CSR_MSTATUS) begin
                mstatus <= wdata&32'h803FFFFF; //[30:22]不变
            end
            else if(exp_valid)
                mstatus <= {mstatus[31:12],mstatus[8:0],3'b110};
            else if(mret_wb)
                mstatus <= {mstatus[31:12],3'b001,mstatus[11:3]};
        end
    end

    reg [31:0] mtvec;
    always_ff @(posedge clk) begin
        if(!rstn) begin
            mtvec <= 32'h0;
        end
        // Lab4 TODO: implement mtvec
        else begin
            if(we && waddr == `CSR_MTVEC) begin
                mtvec <= wdata &32'hFFFFFFFC; //末两位00
            end
        end
    end

    reg [31:0] mcause;
    always_ff @(posedge clk) begin
        if(!rstn) begin
            mcause <= 32'h0;
        end
        // Lab4 TODO: implement mcause
        else begin
            if(we && waddr == `CSR_MCAUSE) begin
                mcause <= wdata;
            end
            else if(exp_valid)
                mcause[3:0]<=exp_code;
        end
    end

    reg [31:0] mepc;
    always_ff @(posedge clk) begin
        if(!rstn) begin
            mepc <= 32'h0;
        end
        // Lab4 TODO: implement mepc
        else begin
            if(we && waddr == `CSR_MEPC) begin
                mepc <= wdata;
            end
            else if(exp_valid)
                mepc <= pc_wb;
        end
    end

    // read
    always_comb begin
        case(raddr)
            `CSR_MSTATUS: rdata = mstatus;
            `CSR_MTVEC  : rdata = mtvec;
            `CSR_MCAUSE : rdata = mcause;
            `CSR_MEPC   : rdata = mepc;
            default     : rdata = 32'h0;
        endcase
    end
    initial begin
        set_csr_ptr(mstatus, mtvec, mepc, mcause);
    end
    assign mtvec_global = mtvec;
    assign mepc_global = mepc;
endmodule