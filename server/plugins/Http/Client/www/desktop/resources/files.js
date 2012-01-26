var tmp_files_toto = 1;

function Files(task)
{
    task.content.firstChild.innerHTML = "<br><br><br><center><h1>[[[[[[[ " + (tmp_files_toto++) + " ]]]]]]]</h1></center>";
}

function initialize_files(task) { return new Files(task); }
gl_resources.jsLoaded("files");
