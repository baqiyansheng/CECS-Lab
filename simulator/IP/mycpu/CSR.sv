`include "./include/config.sv"
module CSR(
    input  logic [ 0:0] clk,
    input  logic [ 0:0] rstn,
    input  logic [11:0] raddr,
    input  logic [11:0] waddr,
    input  logic [ 0:0] we,
    input  logic [31:0] wdata,
    output logic [31:0] rdata
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
endmodule