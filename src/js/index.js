// Audio control
const audio = document.getElementById("backgroundAudio");
let isPlaying = false;

function toggleAudio() {
    if (isPlaying) {
        audio.pause();
    } else {
        audio.play();
    }
    isPlaying = !isPlaying;
}

// Trailer modal
function openTrailer() {
    const modal = document.getElementById("trailerModal");
    const iframe = modal.querySelector(".trailer-iframe");
    iframe.src = "https://www.youtube.com/embed/YoHD9XEInc0";
    modal.classList.add("active");
    document.body.style.overflow = "hidden"; 
}

function closeTrailer() {
    const modal = document.getElementById("trailerModal");
    const iframe = modal.querySelector(".trailer-iframe");
    iframe.src = "about:blank";
    modal.classList.remove("active");
    document.body.style.overflow = "auto";
}

// Close modal when clicking outside
document
    .querySelector(".trailer-modal")
    .addEventListener("click", (e) => {
        if (e.target.classList.contains("trailer-modal")) {
            closeTrailer();
        }
    });