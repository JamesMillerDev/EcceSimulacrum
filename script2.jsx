var threads = ["I know that one of you did this, I just know it", "37xcalibur", 16, 571, "37xcalibur", "lock",
"I was unfairly banned, but I'd like to forgive and forget", "HelloThomas", 27, 321, "JackOfKings", "normal",
"Defending my honor as a hammer counter", "jumbo27", 5, 861, "CanonSnipe", "normal",
"This is for Erik", "Sciaticadepot", 30, 106, "Jusss", "normal",
"A better understanding of what's going on here", "AbG", 21, 323, "mateo", "moved",
"Understanding my place in the community", "Jeff Packitin", 8, 69, "27gamer", "normal",
"The greatest hammer counter?", "Continuation", 50, 473, "john23", "moved",
"Moderator application denied?", "Muvwv", 37, 99, "Joshy", "normal",
"Abuse of the member locator feature", "startlingabomination", 15, 683, "ExcellentFidelity", "normal",
"This community is tearing itself apart...", "mr_mountain", 14, 330, "R4C5", "normal",
"Someone here is having an affair with my wife", "darkrain37x24", 31, 101, "jurokr", "normal",
"Erik - if you have something to say, then just come out and say it", "fluffy", 29, 587, "Zen", "lock",
"The university I attended is perfectly reputable", "MKXVIIIJ", 16, 323, "Lun", "normal",
"Some of the veteran members here are very arrogant...", "Mr. Nomad", 10, 810, "scotty2", "normal",
"A reminder to be respectful of everyone", "no-spaces-allowed", 7, 879, "Mis", "normal",
"Maybe I should just take my leave of here!", "Countingthemiles", 8, 37, "mr27", "normal",
"There's more to life than hammer counting...", "Neyedermayer", 17, 616, "eyeseaweed", "normal",
"Clarifying a misunderstanding", "mpacagard", 31, 376, "CapeD", "normal",
"If we're all here forever...", "whirlwind511", 29, 781, "XSchool", "moved",
"A note to my LEAST favorite member", "Xyrrox", 13, 45, "justwannahavefun", "normal"];

var doc = app.activeDocument;
var cur_file = app.open(new File("C:\\Users\\capta\\threadtitle.psd"));
for (var i = 0; i < threads.length; i += 6)
{
	var starter_delta = 0;
	for (var j = cur_file.artLayers.length - 1; j >= 0; --j)
	{
		var cur_name = cur_file.artLayers[j].name;
		if (cur_name == "lock" && threads[i + 5] != "lock")
			continue;
		
		if (cur_name == "moved" && threads[i + 5] != "moved")
			continue;
		
		if (cur_name == "normal" && threads[i + 5] != "normal")
			continue;
		
		cur_file.artLayers[j].duplicate(doc, ElementPlacement.INSIDE);
		if (cur_name == "Thread Title")
		{
			cur_file.close();
			app.activeDocument.artLayers[0].textItem.contents = threads[i];
			app.activeDocument.artLayers[0].name = "=sequenceforum-";
			cur_file = app.open(new File("C:\\Users\\capta\\threadtitle.psd"));
		}
		
		if (cur_name == "ThreadStarter")
		{
			cur_file.close();
			var prev = app.activeDocument.artLayers[0].bounds[2];
			app.activeDocument.artLayers[0].textItem.contents = threads[i + 1];
			app.activeDocument.artLayers[0].name = "=" + threads[i + 1] + "1-";
			starter_delta = app.activeDocument.artLayers[0].bounds[2] - prev;
			cur_file = app.open(new File("C:\\Users\\capta\\threadtitle.psd"));
		}
		
		if (cur_name == "moveline")
		{
			cur_file.close();
			app.activeDocument.artLayers[0].translate(starter_delta, 0);
			cur_file = app.open(new File("C:\\Users\\capta\\threadtitle.psd"));
		}
		
		if (cur_name == "posts")
		{
			cur_file.close();
			app.activeDocument.artLayers[0].textItem.contents = threads[i + 2];
			cur_file = app.open(new File("C:\\Users\\capta\\threadtitle.psd"));
		}
		
		if (cur_name == "views")
		{
			cur_file.close();
			app.activeDocument.artLayers[0].textItem.contents = threads[i + 3];
			cur_file = app.open(new File("C:\\Users\\capta\\threadtitle.psd"));
		}
		
		if (cur_name == "LastPoster")
		{
			cur_file.close();
			app.activeDocument.artLayers[0].textItem.contents = threads[i + 4];
			app.activeDocument.artLayers[0].name = "=" + threads[i + 4] + "1-";
			cur_file = app.open(new File("C:\\Users\\capta\\threadtitle.psd"));
		}
	}
	
	cur_file.close();
	for (var j = 0; j < 14; ++j)
	{
		app.activeDocument.artLayers[j].translate(324, 438 + ((i / 6) * 63));
	}
	
	cur_file = app.open(new File("C:\\Users\\capta\\threadtitle.psd"));
}

cur_file.close();
var num_threads = threads.length / 6;
if (num_threads > 9)
{
	var delta = (num_threads - 10) * 64;
	for (var i = 0; i < app.activeDocument.artLayers.length; ++i)
	{
		if (app.activeDocument.artLayers[i].name == "silver" || app.activeDocument.artLayers[i].name == "container" || app.activeDocument.artLayers[i].name == "background")
		{
			var original_height = app.activeDocument.artLayers[i].bounds[3] - app.activeDocument.artLayers[i].bounds[1];
			var percent = (original_height + delta) / original_height * 100.0;
			app.activeDocument.artLayers[i].resize(100, percent, AnchorPosition.TOPLEFT);
		}
		
		if (app.activeDocument.artLayers[i].name[0] == '-')
		{
			app.activeDocument.artLayers[i].translate(0, delta);
			app.activeDocument.artLayers[i].name = app.activeDocument.artLayers[i].name.substr(1);
		}
	}
	
	var height = app.activeDocument.height;
	var width = app.activeDocument.width;
	app.activeDocument.resizeCanvas(width, height + delta, AnchorPosition.TOPLEFT);
}
