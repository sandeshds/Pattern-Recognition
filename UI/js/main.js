
function converse() {
    window.location.href = "wordlist.html";
}

function skypeCall() {
    window.location = document.getElementById('skypeCall').href;
}

function postToUrl() {
    window.location.href = "message.html"
}

function soundAlarm(){
    var audio = document.getElementById('id1');
    audio.play();
}