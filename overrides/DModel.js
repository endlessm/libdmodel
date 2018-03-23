const Json = imports.gi.Json;
const GLib = imports.gi.GLib;
const GObject = imports.gi.GObject;

function toUnderscore (string) {
    return string.replace(/-/g, '_');
}

function define_property (klass, name, descriptor) {
    Object.defineProperty(klass.prototype, name, descriptor);
    Object.defineProperty(klass.prototype, toUnderscore(name), descriptor);
}

function marshal_property (props, name, marshaller) {
    if (props[name])
        props[name] = marshaller(props[name]);
    let underscored = toUnderscore(name);
    if (name === underscored)
        return;
    if (props[underscored])
        props[underscored] = marshaller(props[underscored]);
}

function add_custom_model_constructors (model) {
    model.new_from_json = function (object) {
        let json_node = Json.from_string(JSON.stringify(object));
        return model.new_from_json_node(json_node);
    };
    // FIXME: would be way nicer to put this on the actual gobject init, or
    // better yet support introspection on list properties
    model.new_from_props = function (props={}) {
        marshal_property(props, 'discovery-feed-content', function (v) {
            return Json.from_string(JSON.stringify(v)).get_object();
        });
        marshal_property(props, 'table-of-contents', function (v) {
            // Mimic json_gvariant_deserialize
            let toc = [];
            for (let item of v) {
                let new_item = {};
                for (let prop in item) {
                    if (prop === 'hasIndex') {
                        new_item[prop] = new GLib.Variant('i', item[prop]);
                    }
                    else
                        new_item[prop] = new GLib.Variant('s', item[prop]);
                }
                toc.push(new_item);
            }
            return new GLib.Variant('aa{sv}', toc);
        });
        return new model(props);
    };
}

function _init() {
    let DModel = this;

    define_property(DModel.Content, 'discovery-feed-content', {
        get: function () {
            let node = new Json.Node();
            node.init_object(this.get_discovery_feed_content());
            return JSON.parse(Json.to_string(node, false));
        }
    });

    define_property(DModel.Article, 'table-of-contents', {
        get: function () {
            let toc = this.get_table_of_contents();
            if (!toc)
                return [];
            toc = toc.deep_unpack();
            toc.forEach(item => {
                for (let prop in item)
                    item[prop] = item[prop].deep_unpack();
            });
            return toc;
        },
    });

    add_custom_model_constructors(DModel.Audio);
    add_custom_model_constructors(DModel.Content);
    add_custom_model_constructors(DModel.DictionaryEntry);
    add_custom_model_constructors(DModel.Article);
    add_custom_model_constructors(DModel.Set);
    add_custom_model_constructors(DModel.Media);
    add_custom_model_constructors(DModel.Image);
    add_custom_model_constructors(DModel.Video);

    DModel.Query.new_from_object = function (source, props={}) {
        for (let param_spec of GObject.Object.list_properties.call(DModel.Query)) {
            let name = param_spec.name;
            if (props.hasOwnProperty(name))
                continue;
            if (props.hasOwnProperty(toUnderscore(name)))
                continue;
            props[name] = source[name];
        }
        return new DModel.Query(props);
    };

    // FIXME: Can be removed when GJS supports pointer-valued properties.
    define_property(DModel.QueryResults, 'models', {
        get: function () {
            return this.get_models();
        },
    });

    DModel.Query.prototype.toString = DModel.Query.prototype.to_string;
}
