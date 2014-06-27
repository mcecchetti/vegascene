
vegascene_template = function(vs, outSceneGraph, specContent, render) {
    vs["run"] = function() {
        vs[outSceneGraph] = 0;
        var spec = JSON.parse(vs[specContent]);
        vs[render](spec);
    }
}
