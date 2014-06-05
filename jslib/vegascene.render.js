
vegascene_template = function(vs, render, outSceneGraph) {
    vs[render] = function(spec) {
        vg.headless.render(
            {spec: spec, renderer: 'scene'},
            function(err, data) {
                if (err) throw err;
                           vs[outSceneGraph] = data.scene;
            }
        );
    }
};
