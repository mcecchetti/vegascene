
if (d3 !== undefined) {
    if (d3.svg === undefined) {
       function d3_svg() {};
       var prot = d3_svg.prototype;
       prot.self = function() { return this };
       prot.arc = prot.area = prot.line = prot.symbol
           = prot.x = prot.y = prot.y0 = prot.y1
           = prot.type = prot.size = prot.self;
       d3.svg = new d3_svg();
    }
}
