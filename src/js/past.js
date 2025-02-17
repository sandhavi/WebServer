// Sort movies functionality
function sortMovies() {
    const container = document.getElementById("movie-container");
    const sortBy = document.getElementById("sort-by").value;
    const cards = Array.from(
        container.getElementsByClassName("movie-card")
    );

    cards.sort((a, b) => {
        if (sortBy === "date") {
            return new Date(a.dataset.release) - new Date(b.dataset.release);
        } else if (sortBy === "name") {
            return a.dataset.title.localeCompare(b.dataset.title);
        } else if (sortBy === "rating") {
            return parseFloat(b.dataset.rating) - parseFloat(a.dataset.rating);
        }
    });

    // Clear and re-append sorted cards
    while (container.firstChild) {
        container.removeChild(container.firstChild);
    }

    cards.forEach((card) => {
        container.appendChild(card);
    });
}

// Search movies functionality
function searchMovies() {
    const searchTerm = document
        .querySelector(".search-bar input")
        .value.toLowerCase();
    const cards = document.querySelectorAll(".movie-card");

    cards.forEach((card) => {
        const title = card.dataset.title.toLowerCase();
        if (title.includes(searchTerm)) {
            card.style.display = "block";
        } else {
            card.style.display = "none";
        }
    });
}

// Newsletter subscription
function submitNewsletter(event) {
    event.preventDefault();
    const email = event.target.querySelector("input").value;
    alert(`Thank you! You've subscribed with ${email}`);
    event.target.reset();
}