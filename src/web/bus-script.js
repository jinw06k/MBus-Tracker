let autoRefreshInterval;

function updateCurrentTime() {
  const now = new Date();
  const timeString = now.toLocaleTimeString();
  document.getElementById(
    "current-time"
  ).innerText = `Current Time: ${timeString}`;
}

async function fetchPredictions(routeId) {
  const container = document.getElementById("predictions-container");
  const loader = document.getElementById("loader");
  const busNameMap = {
    BB: "Bursley Baits",
    NW: "Northwood",
    NX: "Northwood Express",
    CN: "Commuter North",
    CS: "Commuter South",
    DD: "Diag-to-Diag",
    OS: "Oxford Shuttle",
  };

  loader.style.display = "inline-block";

  container.innerHTML = `<p>Loading predictions for route ${routeId}...</p>`;

  try {
    const response = await fetch(`/.netlify/functions/bus-prediction?route_id=${routeId}`);
    const data = await response.json();
    container.innerHTML = "";

    const table = document.createElement("table");
    table.innerHTML = `
                <thead>
                  <tr>
                    <th>Stop Name</th>
                    <th>Direction</th>
                    <th>${routeId === "ALL" ? "Route ID" : "Vehicle ID"}</th>
                    <th>Arrival Time (min)</th>
                  </tr>
                </thead>
                <tbody>
                  ${
                    data.length > 0
                      ? data
                          .map((prediction) => {
                            const arrivalTime = parseInt(
                              prediction.arrival_time
                            );
                            let style = "";

                            if (arrivalTime >= 10) {
                              style = "color: green; font-weight: bold;";
                            } else if (arrivalTime > 5) {
                              style = "color: orange; font-weight: bold;";
                            } else {
                              style = "color: red; font-weight: bold;";
                            }

                            return `
                              <tr>
                                <td>${prediction.stop_name}</td>
                                <td>${prediction.direction}</td>
                                <td>${
                                  routeId === "ALL"
                                    ? prediction.route_id
                                    : prediction.vehicle_id
                                }</td>
                                <td style="${style}">${arrivalTime}</td>
                              </tr>
                            `;
                          })
                          .join("")
                      : `<tr>
                           <td colspan="4" style="text-align: center; font-weight: bold; color: gray;">
                             No predictions available for this route.
                           </td>
                         </tr>`
                  }
                </tbody>
              `;
    container.appendChild(table);
  } catch (error) {
    container.innerHTML = `<p>Error fetching predictions: ${error.message}</p>`;
  } finally {
    loader.style.display = "none";
  }
}

function startAutoRefresh(routeId) {
  // clear existing interval
  if (autoRefreshInterval) {
    clearInterval(autoRefreshInterval);
  }

  // 15 seconds refresh interval for ALL routes setting, 5 sec otherwise
  const refreshInterval = routeId === "ALL" ? 15000 : 5000;

  fetchPredictions(routeId);
  autoRefreshInterval = setInterval(
    () => fetchPredictions(routeId),
    refreshInterval
  );
}

const buttons = document.querySelectorAll("button");
buttons.forEach((button) => {
  button.addEventListener("click", () => {
    buttons.forEach((btn) => btn.classList.remove("last-clicked"));
    button.classList.add("last-clicked");

    const routeId = button.textContent.trim();
    startAutoRefresh(routeId);
  });
});

updateCurrentTime();
setInterval(updateCurrentTime, 1000);
