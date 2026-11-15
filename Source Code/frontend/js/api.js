// API client wrapper and session manager for all four system interfaces
const apiBaseUrl = (window.location.origin && window.location.origin.startsWith('http')) 
  ? window.location.origin 
  : 'http://localhost:8080';

const api = {
  // Generic GET request handler
  async get(endpoint) {
    try {
      const response = await fetch(`${apiBaseUrl}${endpoint}`);
      return await response.json();
    } catch (error) {
      console.error(`GET ${endpoint} error:`, error);
      return { success: false, message: 'Cannot connect to C++ backend server!' };
    }
  },

  // Generic POST request handler
  async post(endpoint, data) {
    try {
      const response = await fetch(`${apiBaseUrl}${endpoint}`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(data)
      });
      return await response.json();
    } catch (error) {
      console.error(`POST ${endpoint} error:`, error);
      return { success: false, message: 'Cannot connect to C++ backend server!' };
    }
  },

  // Retrieve current logged in user from localStorage
  getUser() {
    const rawUserData = localStorage.getItem('vrs_user');
    return rawUserData ? JSON.parse(rawUserData) : null;
  },

  // Store current logged in user to localStorage
  setUser(userData) {
    localStorage.setItem('vrs_user', JSON.stringify(userData));
  },

  // Clear session and return to login page
  logout() {
    localStorage.removeItem('vrs_user');
    window.location.href = 'index.html';
  },

  // Guard access based on user role
  checkAuth(allowedRoles) {
    const user = this.getUser();
    if (!user) {
      window.location.href = 'index.html';
      return null;
    }
    if (allowedRoles && !allowedRoles.includes(user.role)) {
      alert(`Unauthorized access! Required role: ${allowedRoles.join(', ')}.`);
      this.logout();
      return null;
    }
    return user;
  },

  // Currency formatting helper
  formatMoney(amount) {
    if (!amount) return '0 ₫';
    return new Intl.NumberFormat('vi-VN', { style: 'currency', currency: 'VND' }).format(amount);
  }
};
