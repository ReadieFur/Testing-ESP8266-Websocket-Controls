window.addEventListener("load", () =>
{
    //document.body.appendChild(document.createElement("hr"));
    let documentHTML = '"' + document.documentElement.innerHTML
        .replaceAll(/<!-- Code injected by live-server -->([\S\s]*?)<\/script>/gm, "") //Remove LiveServer JS
        .replaceAll(/<script id="cppjs"([\S\s]*?)<\/script>/gm, "") //Remove this script
        .replaceAll(`"`, `'`)
        .replaceAll("cpp='", '" + ')
        .replaceAll(" cpp'", ' + " ')
        .replaceAll("/*cpp", '" + ')
        .replaceAll("cpp*/", ' + " ');
    let htmlCPPmatch = documentHTML.match(/" +(.*?)\+ "/g);
    if (htmlCPPmatch != null) { htmlCPPmatch.forEach(cpp => { documentHTML = documentHTML.replace(cpp, cpp.replaceAll("`", '"')); }) } //Replace CPP segments that have ` with "
    //https://stackoverflow.com/questions/23284784/javascript-minify-html-regex
    documentHTML = documentHTML
        .replaceAll(/<!--([\S\s]*?)-->/gm, "") //Remove HTML comments
        .replaceAll(/\/\*(.|\n)*?\*\//gm, "") //Remove JS/CSS comments
        .replaceAll(/\>[\r\n ]+\</g, "><") //Minify HTML
        .replaceAll(/(<.*?>)|\s+/g, (m, $1) => $1 ? $1 : ' ') + '"'; //minify CSS/JS
    console.log(documentHTML);
    let minified = document.createElement("textarea");
    minified.value = documentHTML;
    minified.readOnly = true;
    minified.style = "min-width: 100%; max-width: 100%; min-height: 200px";
    document.body.appendChild(minified);
})