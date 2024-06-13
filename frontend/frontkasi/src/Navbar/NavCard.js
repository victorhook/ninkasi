import './NavCard.css';

function NavCard({ title, value }) {
  return (
    <div className="card navcard d-flex text-center">
        <span className='fs-6 navcard-title'>{ title }</span>
        <hr className='m-0' />
        <span className='fs-4 navcard-value'>{ value }</span>
    </div>
  );
}

export default NavCard;
