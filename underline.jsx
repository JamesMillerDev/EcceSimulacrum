var doc = app.activeDocument;
for (var i = 0; i < doc.artLayers.length; ++i)
{
	if (doc.artLayers[i].name[0] == ';' || doc.artLayers[i].name[0] == '=' || doc.artLayers[i].name[0] == '[' || doc.artLayers[i].name[0] == ']' || doc.artLayers[i].name[0] == '&')
	{
		var underline_char = doc.artLayers[i].name[0];
		if (doc.artLayers[i].name[2] != underline_char)
		{
			var cur_layer = doc.artLayers[i];
			var new_layer = cur_layer.duplicate();
			new_layer.textItem.underline = UnderlineType.UNDERLINELEFT;
			var delta = new_layer.bounds[3] - cur_layer.bounds[3];
			cur_layer.name = underline_char + delta.toString()[0] + underline_char + cur_layer.name.substr(1);
			new_layer.remove();
		}
	}
}