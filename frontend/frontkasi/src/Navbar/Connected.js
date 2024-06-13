import './Height.css';

function Connected({ status }) {
  return (
    <p className="card connected p-2 m-2">
      { status }
    </p>
  );
}

export default Connected;
